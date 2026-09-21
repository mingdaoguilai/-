from maix import camera, display, image, nn
from maix import uart, pinmap, err, app

from http.server import BaseHTTPRequestHandler, HTTPServer
import socketserver
import socket
import threading


# ============================================================
# User-adjustable settings
# ============================================================

MODEL_PATH = "/root/models/model_305994.mud"

# The camera/display/stream image is 4:3.
# The YOLO model remains 448x448. detector.detect() letterboxes the
# 640x480 image to the square model input while preserving its ratio.
FRAME_WIDTH = 640
FRAME_HEIGHT = 480

CONF_TH = 0.65
IOU_TH = 0.45

# None: detect the complete 640x480 image.
# Example 4:3 center ROI: (80, 60, 480, 360)
DETECTION_ROI = None

# UART output interval.
SEND_EVERY_N_FRAMES = 1

# Question 3 position calibration. Replace these two image points with
# the physical ends of the 25 cm beam in the installed camera image.
AXIS_LEFT = (80, 240)
AXIS_RIGHT = (560, 240)
AXIS_LENGTH_MM = 250.0
POSITION_SIGN = 1

# Optional class filters and display names.
BALL_CLASS_ID = 0
ENABLED_CLASS_IDS = [BALL_CLASS_ID]
CUSTOM_LABELS = {
    BALL_CLASS_ID: "ball",
}


# ============================================================
# Ball-center refinement settings
# ============================================================

ENABLE_CIRCLE_REFINEMENT = True

# Expand each YOLO box before Canny/Hough processing.
REFINE_ROI_PADDING_RATIO = 0.25
REFINE_ROI_MIN_PADDING = 5

# ROI noise reduction. Median filtering is effective but relatively
# slow. Leave it False first when low delay is important.
USE_MEDIAN_FILTER = False
MEDIAN_FILTER_SIZE = 1       # 1 = 3x3

USE_GAUSSIAN_FILTER = True
GAUSSIAN_FILTER_SIZE = 1     # 1 = 3x3

CANNY_LOW = 50
CANNY_HIGH = 120

# Canny-edge Hough is attempted first.
HOUGH_EDGE_THRESHOLD = 100

# If Canny breaks the circle edge, retry on the denoised grayscale ROI.
USE_GRAY_HOUGH_FALLBACK = True
HOUGH_GRAY_THRESHOLD = 1800

HOUGH_X_STRIDE = 2
HOUGH_Y_STRIDE = 2
HOUGH_R_STEP = 1

MIN_RADIUS_RATIO = 0.45
MAX_RADIUS_RATIO = 1.35
MAX_CENTER_OFFSET_RATIO = 0.60


# ============================================================
# Low-delay temporal tracking
# ============================================================

MEDIAN_WINDOW_SIZE = 3
FILTER_ALPHA_SLOW = 0.30
FILTER_ALPHA_FAST = 0.85
FAST_TRACK_DISTANCE = 12

MAX_JUMP_PIXELS = 120
CONFIRM_FRAMES = 1
MAX_MISSED_FRAMES = 4


# ============================================================
# Display settings
# ============================================================

BOX_COLOR = image.COLOR_RED
BOX_THICKNESS = 2

RAW_CENTER_COLOR = image.COLOR_BLUE
REFINED_COLOR = image.COLOR_GREEN
FILTERED_COLOR = image.COLOR_YELLOW

CENTER_RADIUS = 4
TEXT_COLOR = image.COLOR_RED
TEXT_SCALE = 1.0
TEXT_OFFSET_Y = 20

SHOW_DETECTION_ROI = True
SHOW_REFINEMENT_ROI = True
SHOW_RAW_CENTER = True
SHOW_FILTERED_CENTER = True


# ============================================================
# MJPEG streaming settings
# ============================================================

STREAM_ENABLED = True
STREAM_HOST = "0.0.0.0"
STREAM_PORT = 8000

# Lower quality and publishing every second frame reduce CPU load and
# keep YOLO/control latency low.
STREAM_JPEG_QUALITY = 65
STREAM_EVERY_N_FRAMES = 2


# ============================================================
# Shared streaming state
# ============================================================

latest_jpeg = None
latest_frame_id = 0
stream_condition = threading.Condition()
stream_stop_event = threading.Event()


HTML_PAGE = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>钢球坐标识别与图传系统</title>
    <style>
        * { box-sizing: border-box; }
        body {
            margin: 0;
            padding: 20px;
            color: #ffffff;
            background: #1f2937;
            font-family: Arial, "Microsoft YaHei", sans-serif;
            text-align: center;
        }
        .container {
            width: min(920px, 100%);
            margin: 0 auto;
            padding: 20px;
            border-radius: 14px;
            background: #334155;
            box-shadow: 0 8px 24px rgba(0, 0, 0, 0.30);
        }
        .video-wrap {
            width: 100%;
            aspect-ratio: 4 / 3;
            overflow: hidden;
            border-radius: 10px;
            background: #000000;
        }
        #videoStream {
            display: block;
            width: 100%;
            height: 100%;
            object-fit: contain;
        }
        .buttons {
            display: flex;
            gap: 12px;
            margin-top: 16px;
        }
        button {
            flex: 1;
            padding: 12px;
            border: 0;
            border-radius: 8px;
            color: white;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
        }
        .start { background: #16a34a; }
        .stop { background: #dc2626; }
        button:disabled {
            background: #64748b;
            cursor: not-allowed;
        }
        #status {
            margin-top: 14px;
            color: #cbd5e1;
        }
    </style>
</head>
<body>
    <main class="container">
        <h2>车载平衡滚动球——图传与识别系统</h2>
        <div class="video-wrap">
            <img id="videoStream" src="/video_feed" alt="等待图传画面">
        </div>
        <div class="buttons">
            <button id="startBtn" class="start" onclick="startRecording()">
                开始录制
            </button>
            <button id="stopBtn" class="stop" onclick="stopRecording()" disabled>
                停止并保存
            </button>
        </div>
        <div id="status">等待画面连接</div>
    </main>

    <script>
        let mediaRecorder = null;
        let recordedChunks = [];
        let canvasTimer = null;

        const videoImage = document.getElementById("videoStream");
        const startBtn = document.getElementById("startBtn");
        const stopBtn = document.getElementById("stopBtn");
        const statusText = document.getElementById("status");

        videoImage.onload = () => {
            if (!mediaRecorder || mediaRecorder.state === "inactive") {
                statusText.textContent = "图传已连接：640×480（4:3）";
            }
        };

        function startRecording() {
            if (!window.MediaRecorder) {
                statusText.textContent = "当前浏览器不支持 MediaRecorder";
                return;
            }

            recordedChunks = [];

            const canvas = document.createElement("canvas");
            canvas.width = 640;
            canvas.height = 480;
            const ctx = canvas.getContext("2d");

            canvasTimer = setInterval(() => {
                try {
                    ctx.drawImage(videoImage, 0, 0, 640, 480);
                } catch (_) {
                }
            }, 33);

            const stream = canvas.captureStream(30);
            let options = {};

            if (MediaRecorder.isTypeSupported("video/webm;codecs=vp8")) {
                options = { mimeType: "video/webm;codecs=vp8" };
            } else if (MediaRecorder.isTypeSupported("video/webm")) {
                options = { mimeType: "video/webm" };
            }

            mediaRecorder = new MediaRecorder(stream, options);

            mediaRecorder.ondataavailable = (event) => {
                if (event.data && event.data.size > 0) {
                    recordedChunks.push(event.data);
                }
            };

            mediaRecorder.onstop = () => {
                clearInterval(canvasTimer);
                canvasTimer = null;

                const blob = new Blob(recordedChunks, {
                    type: mediaRecorder.mimeType || "video/webm"
                });
                const url = URL.createObjectURL(blob);
                const link = document.createElement("a");

                link.href = url;
                link.download = `ball_video_${Date.now()}.webm`;
                document.body.appendChild(link);
                link.click();
                link.remove();

                setTimeout(() => URL.revokeObjectURL(url), 1000);
                statusText.textContent = "录制完成，视频已保存到电脑";
                statusText.style.color = "#22c55e";
            };

            mediaRecorder.start();
            startBtn.disabled = true;
            stopBtn.disabled = false;
            statusText.textContent = "正在录制视频";
            statusText.style.color = "#ef4444";
        }

        function stopRecording() {
            if (mediaRecorder && mediaRecorder.state !== "inactive") {
                mediaRecorder.stop();
            }
            startBtn.disabled = false;
            stopBtn.disabled = true;
        }
    </script>
</body>
</html>
"""


class CamHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/" or self.path.startswith("/?"):
            page = HTML_PAGE.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(page)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(page)
            return

        if self.path == "/video_feed":
            self.send_response(200)
            self.send_header(
                "Content-Type",
                "multipart/x-mixed-replace; boundary=frame",
            )
            self.send_header("Cache-Control", "no-store, no-cache")
            self.send_header("Pragma", "no-cache")
            self.send_header("Connection", "close")
            self.end_headers()

            last_frame_id = -1

            try:
                while not stream_stop_event.is_set():
                    with stream_condition:
                        if latest_frame_id == last_frame_id:
                            stream_condition.wait(timeout=1.0)

                        jpeg_data = latest_jpeg
                        frame_id = latest_frame_id

                    if jpeg_data is None or frame_id == last_frame_id:
                        continue

                    last_frame_id = frame_id

                    header = (
                        b"--frame\r\n"
                        b"Content-Type: image/jpeg\r\n"
                        + (
                            "Content-Length: %d\r\n\r\n"
                            % len(jpeg_data)
                        ).encode("ascii")
                    )

                    self.wfile.write(header)
                    self.wfile.write(jpeg_data)
                    self.wfile.write(b"\r\n")
                    self.wfile.flush()
            except (
                BrokenPipeError,
                ConnectionResetError,
                ConnectionAbortedError,
            ):
                pass
            except Exception as e:
                print("MJPEG client ended:", e)

            return

        self.send_error(404)

    def log_message(self, format_text, *args):
        # Suppress per-request logs to keep the serial console readable.
        pass


class ThreadedHTTPServer(socketserver.ThreadingMixIn, HTTPServer):
    daemon_threads = True
    allow_reuse_address = True


http_server = None


def start_stream_server():
    global http_server

    if not STREAM_ENABLED:
        return

    try:
        http_server = ThreadedHTTPServer(
            (STREAM_HOST, STREAM_PORT),
            CamHandler,
        )

        thread = threading.Thread(
            target=http_server.serve_forever,
            daemon=True,
        )
        thread.start()

        try:
            local_ip = socket.gethostbyname(socket.gethostname())
        except Exception:
            local_ip = "<MaixCAM2-IP>"

        print(
            "MJPEG stream ready:",
            "http://%s:%d" % (local_ip, STREAM_PORT),
        )
    except Exception as e:
        http_server = None
        print("MJPEG server initialization failed:", e)


def stop_stream_server():
    stream_stop_event.set()

    with stream_condition:
        stream_condition.notify_all()

    if http_server is not None:
        try:
            http_server.shutdown()
            http_server.server_close()
        except Exception as e:
            print("MJPEG server shutdown failed:", e)


def publish_stream_frame(img):
    global latest_jpeg
    global latest_frame_id

    try:
        jpeg_img = img.to_jpeg(STREAM_JPEG_QUALITY)
        jpeg_data = jpeg_img.to_bytes()

        # Detach from the temporary Maix image when conversion is
        # supported. Otherwise the Maix byte buffer itself is retained.
        try:
            jpeg_data = bytes(jpeg_data)
        except Exception:
            pass

        with stream_condition:
            latest_jpeg = jpeg_data
            latest_frame_id += 1
            stream_condition.notify_all()
    except Exception as e:
        print("JPEG publish failed:", e)


# ============================================================
# General helpers
# ============================================================

def clamp(value, minimum, maximum):
    if value < minimum:
        return minimum
    if value > maximum:
        return maximum
    return value


def project_ball_position_0p1mm(x, y):
    axis_x = AXIS_RIGHT[0] - AXIS_LEFT[0]
    axis_y = AXIS_RIGHT[1] - AXIS_LEFT[1]
    axis_length_sq = axis_x * axis_x + axis_y * axis_y

    if axis_length_sq <= 0:
        return 0

    ratio = (
        (x - AXIS_LEFT[0]) * axis_x
        + (y - AXIS_LEFT[1]) * axis_y
    ) / float(axis_length_sq)

    position_mm = (
        (ratio - 0.5)
        * AXIS_LENGTH_MM
        * POSITION_SIGN
    )
    return int(position_mm * 10.0 + (0.5 if position_mm >= 0 else -0.5))


def axis_point_for_position_mm(position_mm):
    ratio = 0.5 + (
        position_mm * POSITION_SIGN / AXIS_LENGTH_MM
    )
    x = int(
        AXIS_LEFT[0]
        + ratio * (AXIS_RIGHT[0] - AXIS_LEFT[0])
        + 0.5
    )
    y = int(
        AXIS_LEFT[1]
        + ratio * (AXIS_RIGHT[1] - AXIS_LEFT[1])
        + 0.5
    )
    return x, y


def draw_position_marker(img, position_mm, color):
    x, y = axis_point_for_position_mm(position_mm)
    axis_x = AXIS_RIGHT[0] - AXIS_LEFT[0]
    axis_y = AXIS_RIGHT[1] - AXIS_LEFT[1]
    norm = max(1.0, (axis_x * axis_x + axis_y * axis_y) ** 0.5)
    normal_x = int(-axis_y * 18.0 / norm)
    normal_y = int(axis_x * 18.0 / norm)

    img.draw_line(
        x - normal_x,
        y - normal_y,
        x + normal_x,
        y + normal_y,
        color,
        thickness=2,
    )


def make_control_packet(sequence, position_0p1mm, confidence, valid):
    value = clamp(int(position_0p1mm), -32768, 32767)
    if value < 0:
        value += 65536

    packet = bytearray(8)
    packet[0] = 0xAA
    packet[1] = 0x55
    packet[2] = sequence & 0xFF
    packet[3] = value & 0xFF
    packet[4] = (value >> 8) & 0xFF
    packet[5] = clamp(int(confidence), 0, 100)
    packet[6] = 1 if valid else 0

    checksum = 0
    for item in packet[:7]:
        checksum ^= item
    packet[7] = checksum
    return bytes(packet)


def median_value(values):
    ordered = sorted(values)
    count = len(ordered)

    if count == 0:
        return 0.0

    middle = count // 2

    if count % 2:
        return float(ordered[middle])

    return (ordered[middle - 1] + ordered[middle]) * 0.5


def get_detection_image(img):
    """
    Return the full frame, or crop the fixed recognition ROI.

    YOLO coordinates are relative to the returned image. offset_x and
    offset_y convert those coordinates back to the 640x480 frame.
    """
    if DETECTION_ROI is None:
        return img, 0, 0, (0, 0, FRAME_WIDTH, FRAME_HEIGHT)

    x = clamp(DETECTION_ROI[0], 0, FRAME_WIDTH - 1)
    y = clamp(DETECTION_ROI[1], 0, FRAME_HEIGHT - 1)
    right = clamp(
        x + DETECTION_ROI[2],
        x + 1,
        FRAME_WIDTH,
    )
    bottom = clamp(
        y + DETECTION_ROI[3],
        y + 1,
        FRAME_HEIGHT,
    )

    w = right - x
    h = bottom - y

    return img.crop(x, y, w, h), x, y, (x, y, w, h)


def make_refine_roi(x, y, w, h):
    pad_x = max(
        REFINE_ROI_MIN_PADDING,
        int(w * REFINE_ROI_PADDING_RATIO),
    )
    pad_y = max(
        REFINE_ROI_MIN_PADDING,
        int(h * REFINE_ROI_PADDING_RATIO),
    )

    roi_x = clamp(x - pad_x, 0, FRAME_WIDTH - 1)
    roi_y = clamp(y - pad_y, 0, FRAME_HEIGHT - 1)
    right = clamp(x + w + pad_x, roi_x + 1, FRAME_WIDTH)
    bottom = clamp(y + h + pad_y, roi_y + 1, FRAME_HEIGHT)

    return roi_x, roi_y, right - roi_x, bottom - roi_y


def find_best_circle(
    circles,
    expected_x,
    expected_y,
    expected_radius,
    max_offset,
):
    best_circle = None
    best_cost = None
    max_offset_sq = max_offset * max_offset

    for circle in circles:
        dx = circle.x() - expected_x
        dy = circle.y() - expected_y
        center_distance_sq = dx * dx + dy * dy

        if center_distance_sq > max_offset_sq:
            continue

        radius_error = circle.r() - expected_radius

        cost = (
            center_distance_sq
            + radius_error * radius_error * 2.0
            - circle.magnitude() * 0.01
        )

        if best_circle is None or cost < best_cost:
            best_circle = circle
            best_cost = cost

    return best_circle


def run_hough(source_img, threshold, r_min, r_max):
    try:
        return source_img.find_circles(
            roi=[],
            x_stride=HOUGH_X_STRIDE,
            y_stride=HOUGH_Y_STRIDE,
            threshold=threshold,
            x_margin=5,
            y_margin=5,
            r_margin=3,
            r_min=r_min,
            r_max=r_max,
            r_step=HOUGH_R_STEP,
        )
    except Exception as e:
        print("Hough detection failed:", e)
        return []


def refine_ball_center(img, x, y, w, h):
    if not ENABLE_CIRCLE_REFINEMENT or w < 6 or h < 6:
        return None

    roi_x, roi_y, roi_w, roi_h = make_refine_roi(x, y, w, h)

    if roi_w < 8 or roi_h < 8:
        return None

    # This runs before any graphics are drawn onto img.
    roi_img = img.crop(roi_x, roi_y, roi_w, roi_h)

    try:
        gray = roi_img.to_format(image.Format.FMT_GRAYSCALE)
    except Exception as e:
        print("Grayscale conversion failed:", e)
        return None

    if USE_MEDIAN_FILTER:
        try:
            gray.median(MEDIAN_FILTER_SIZE, percentile=0.5)
        except Exception as e:
            print("Median filter failed:", e)

    if USE_GAUSSIAN_FILTER:
        try:
            gray.gaussian(GAUSSIAN_FILTER_SIZE)
        except Exception as e:
            print("Gaussian filter failed:", e)

    expected_x = x + w // 2 - roi_x
    expected_y = y + h // 2 - roi_y
    expected_radius = max(2, int((w + h) / 4))

    r_min = max(2, int(expected_radius * MIN_RADIUS_RATIO))
    r_max = max(r_min + 1, int(expected_radius * MAX_RADIUS_RATIO))
    r_max = min(r_max, max(2, min(roi_w, roi_h) // 2))

    if r_min >= r_max:
        r_min = max(2, r_max - 1)

    max_offset = max(
        5,
        int(min(w, h) * MAX_CENTER_OFFSET_RATIO),
    )

    best_circle = None
    source_name = None

    try:
        edge_img = gray.copy()
        edge_img.find_edges(
            image.EdgeDetector.EDGE_CANNY,
            roi=[],
            threshold=[CANNY_LOW, CANNY_HIGH],
        )

        circles = run_hough(
            edge_img,
            HOUGH_EDGE_THRESHOLD,
            r_min,
            r_max,
        )
        best_circle = find_best_circle(
            circles,
            expected_x,
            expected_y,
            expected_radius,
            max_offset,
        )

        if best_circle is not None:
            source_name = "CANNY"
    except Exception as e:
        print("Canny processing failed:", e)

    if best_circle is None and USE_GRAY_HOUGH_FALLBACK:
        circles = run_hough(
            gray,
            HOUGH_GRAY_THRESHOLD,
            r_min,
            r_max,
        )
        best_circle = find_best_circle(
            circles,
            expected_x,
            expected_y,
            expected_radius,
            max_offset,
        )

        if best_circle is not None:
            source_name = "GRAY"

    if best_circle is None:
        return None

    return (
        roi_x + best_circle.x(),
        roi_y + best_circle.y(),
        best_circle.r(),
        source_name,
        (roi_x, roi_y, roi_w, roi_h),
    )


# ============================================================
# Low-delay tracker
# ============================================================

filtered_x = None
filtered_y = None
history_x = []
history_y = []

tracked_class_id = -1
tracked_score = 0.0
tracked_source = "NONE"

detected_frames = 0
missed_frames = 0
track_valid = False


def reset_tracker():
    global filtered_x, filtered_y
    global history_x, history_y
    global tracked_class_id, tracked_score, tracked_source
    global detected_frames, missed_frames, track_valid

    filtered_x = None
    filtered_y = None
    history_x = []
    history_y = []

    tracked_class_id = -1
    tracked_score = 0.0
    tracked_source = "NONE"

    detected_frames = 0
    missed_frames = 0
    track_valid = False


def tracker_missed():
    global missed_frames, detected_frames

    missed_frames += 1
    detected_frames = 0

    if missed_frames > MAX_MISSED_FRAMES:
        reset_tracker()


def update_tracker(class_id, measured_x, measured_y, score, source):
    global filtered_x, filtered_y
    global history_x, history_y
    global tracked_class_id, tracked_score, tracked_source
    global detected_frames, missed_frames, track_valid

    if filtered_x is not None:
        dx = measured_x - filtered_x
        dy = measured_y - filtered_y

        if dx * dx + dy * dy > MAX_JUMP_PIXELS * MAX_JUMP_PIXELS:
            tracker_missed()
            return False

    history_x.append(measured_x)
    history_y.append(measured_y)

    while len(history_x) > MEDIAN_WINDOW_SIZE:
        history_x.pop(0)
    while len(history_y) > MEDIAN_WINDOW_SIZE:
        history_y.pop(0)

    if filtered_x is None:
        filtered_x = float(measured_x)
        filtered_y = float(measured_y)
    else:
        dx = measured_x - filtered_x
        dy = measured_y - filtered_y
        fast_sq = FAST_TRACK_DISTANCE * FAST_TRACK_DISTANCE

        if dx * dx + dy * dy >= fast_sq:
            # Fast movement: bypass old median samples to reduce delay.
            target_x = float(measured_x)
            target_y = float(measured_y)
            alpha = FILTER_ALPHA_FAST
            history_x = [measured_x]
            history_y = [measured_y]
        else:
            # Nearly stationary: median + slow EMA suppresses jitter.
            target_x = median_value(history_x)
            target_y = median_value(history_y)
            alpha = FILTER_ALPHA_SLOW

        filtered_x += alpha * (target_x - filtered_x)
        filtered_y += alpha * (target_y - filtered_y)

    tracked_class_id = class_id
    tracked_score = score
    tracked_source = source

    detected_frames += 1
    missed_frames = 0

    if detected_frames >= CONFIRM_FRAMES:
        track_valid = True

    return True


def choose_primary_detection(detections):
    if not detections:
        return -1

    if filtered_x is None:
        best_index = 0

        for index in range(1, len(detections)):
            if detections[index][8] > detections[best_index][8]:
                best_index = index

        return best_index

    best_index = -1
    best_cost = None

    for index, detection in enumerate(detections):
        dx = detection[6] - filtered_x
        dy = detection[7] - filtered_y
        cost = dx * dx + dy * dy - detection[8] * 1000.0

        if best_index < 0 or cost < best_cost:
            best_index = index
            best_cost = cost

    return best_index


# ============================================================
# Model, 4:3 camera/display, UART and HTTP initialization
# ============================================================

detector = nn.YOLOv5(MODEL_PATH)

MODEL_WIDTH = detector.input_width()
MODEL_HEIGHT = detector.input_height()

print("Model labels:", detector.labels)
print(
    "Model input:",
    MODEL_WIDTH,
    MODEL_HEIGHT,
    detector.input_format(),
)

if MODEL_WIDTH != 448 or MODEL_HEIGHT != 448:
    print(
        "Warning: expected a 448x448 model, but loaded:",
        MODEL_WIDTH,
        MODEL_HEIGHT,
    )

# Camera and output frame are 640x480 (4:3). The model input format is
# retained so detector.detect() can process the frame directly.
cam = camera.Camera(
    FRAME_WIDTH,
    FRAME_HEIGHT,
    detector.input_format(),
)
disp = display.Display()

serial = None
uart_ready = False

try:
    err.check_raise(
        pinmap.set_pin_function("A21", "UART4_TX"),
        "Failed to set A21 as UART4_TX",
    )
    err.check_raise(
        pinmap.set_pin_function("A22", "UART4_RX"),
        "Failed to set A22 as UART4_RX",
    )

    print("UART devices:", uart.list_devices())
    serial = uart.UART("/dev/ttyS4", 115200)
    uart_ready = True
    print("UART4 ready: A21=TX, A22=RX, 115200 8N1")
except Exception as e:
    print("UART4 initialization failed:", e)

start_stream_server()


# ============================================================
# Main loop
# ============================================================

send_frame_counter = 0
stream_frame_counter = 0
uart_sequence = 0

try:
    while not app.need_exit():
        img = cam.read()

        if img is None:
            continue

        detection_img, offset_x, offset_y, detection_roi = (
            get_detection_image(img)
        )

        # FIT_CONTAIN preserves the 4:3 camera image ratio while feeding
        # the 448x448 model. It prevents stretching the ball into an oval.
        objs = detector.detect(
            detection_img,
            conf_th=CONF_TH,
            iou_th=IOU_TH,
            fit=image.Fit.FIT_CONTAIN,
        )

        # class_id, label, x, y, w, h, cx, cy, score
        detections = []

        for obj in objs:
            class_id = obj.class_id

            if (
                ENABLED_CLASS_IDS is not None
                and class_id not in ENABLED_CLASS_IDS
            ):
                continue

            if class_id in CUSTOM_LABELS:
                label = CUSTOM_LABELS[class_id]
            elif 0 <= class_id < len(detector.labels):
                label = detector.labels[class_id]
            else:
                label = str(class_id)

            # obj coordinates are relative to detection_img. Only an
            # offset is needed because detection_img is cropped, not
            # manually resized.
            x = clamp(offset_x + obj.x, 0, FRAME_WIDTH - 1)
            y = clamp(offset_y + obj.y, 0, FRAME_HEIGHT - 1)
            right = clamp(x + obj.w, x + 1, FRAME_WIDTH)
            bottom = clamp(y + obj.h, y + 1, FRAME_HEIGHT)
            w = right - x
            h = bottom - y

            cx = x + w // 2
            cy = y + h // 2

            detections.append(
                (
                    class_id,
                    label,
                    x,
                    y,
                    w,
                    h,
                    cx,
                    cy,
                    obj.score,
                )
            )

        primary_index = choose_primary_detection(detections)
        refined_result = None
        measurement_fresh = False

        if primary_index >= 0:
            primary = detections[primary_index]

            refined_result = refine_ball_center(
                img,
                primary[2],
                primary[3],
                primary[4],
                primary[5],
            )

            if refined_result is not None:
                measured_x = refined_result[0]
                measured_y = refined_result[1]
                measured_source = refined_result[3]
            else:
                measured_x = primary[6]
                measured_y = primary[7]
                measured_source = "YOLO"

            measurement_fresh = update_tracker(
                primary[0],
                measured_x,
                measured_y,
                primary[8],
                measured_source,
            )
        else:
            tracker_missed()

        img.draw_line(
            AXIS_LEFT[0],
            AXIS_LEFT[1],
            AXIS_RIGHT[0],
            AXIS_RIGHT[1],
            image.COLOR_BLUE,
            thickness=1,
        )
        draw_position_marker(img, -50.0, image.COLOR_RED)
        draw_position_marker(img, 0.0, image.COLOR_YELLOW)
        draw_position_marker(img, 50.0, image.COLOR_GREEN)

        # Draw only after Canny/Hough processing, so drawn lines cannot
        # become false edges.
        if SHOW_DETECTION_ROI and DETECTION_ROI is not None:
            img.draw_rect(
                detection_roi[0],
                detection_roi[1],
                detection_roi[2],
                detection_roi[3],
                image.COLOR_GREEN,
                thickness=2,
            )

        for index, detection in enumerate(detections):
            class_id = detection[0]
            label = detection[1]
            x = detection[2]
            y = detection[3]
            w = detection[4]
            h = detection[5]
            raw_cx = detection[6]
            raw_cy = detection[7]
            score = detection[8]

            draw_cx = raw_cx
            draw_cy = raw_cy
            source = "YOLO"

            img.draw_rect(
                x,
                y,
                w,
                h,
                BOX_COLOR,
                thickness=BOX_THICKNESS,
            )

            if SHOW_RAW_CENTER:
                img.draw_circle(
                    raw_cx,
                    raw_cy,
                    2,
                    RAW_CENTER_COLOR,
                    thickness=-1,
                )

            if index == primary_index and refined_result is not None:
                draw_cx = refined_result[0]
                draw_cy = refined_result[1]
                radius = refined_result[2]
                source = refined_result[3]
                refine_roi = refined_result[4]

                img.draw_circle(
                    draw_cx,
                    draw_cy,
                    radius,
                    REFINED_COLOR,
                    thickness=2,
                )
                img.draw_circle(
                    draw_cx,
                    draw_cy,
                    CENTER_RADIUS,
                    REFINED_COLOR,
                    thickness=-1,
                )

                if SHOW_REFINEMENT_ROI:
                    img.draw_rect(
                        refine_roi[0],
                        refine_roi[1],
                        refine_roi[2],
                        refine_roi[3],
                        REFINED_COLOR,
                        thickness=1,
                    )

            text = "%s %.2f %s C(%d,%d)" % (
                label,
                score,
                source,
                draw_cx,
                draw_cy,
            )
            text_y = y - TEXT_OFFSET_Y

            if text_y < 0:
                text_y = y + 2

            img.draw_string(
                x,
                text_y,
                text,
                TEXT_COLOR,
                scale=TEXT_SCALE,
            )

        if (
            track_valid
            and filtered_x is not None
            and SHOW_FILTERED_CENTER
        ):
            output_x = int(filtered_x + 0.5)
            output_y = int(filtered_y + 0.5)
            output_position_0p1mm = project_ball_position_0p1mm(
                output_x,
                output_y,
            )

            img.draw_circle(
                output_x,
                output_y,
                CENTER_RADIUS + 2,
                FILTERED_COLOR,
                thickness=2,
            )
            img.draw_string(
                2,
                2,
                "OUT %s (%d,%d) POS=%+.1fmm" % (
                    tracked_source,
                    output_x,
                    output_y,
                    output_position_0p1mm / 10.0,
                ),
                FILTERED_COLOR,
                scale=1.0,
            )

        send_frame_counter += 1

        if (
            uart_ready
            and send_frame_counter >= SEND_EVERY_N_FRAMES
        ):
            send_frame_counter = 0

            output_valid = (
                measurement_fresh
                and track_valid
                and filtered_x is not None
                and filtered_y is not None
            )
            confidence = clamp(
                int(tracked_score * 100 + 0.5),
                0,
                100,
            ) if output_valid else 0
            position_0p1mm = (
                project_ball_position_0p1mm(
                    filtered_x,
                    filtered_y,
                )
                if output_valid
                else 0
            )

            serial.write(
                make_control_packet(
                    uart_sequence,
                    position_0p1mm,
                    confidence,
                    output_valid,
                )
            )
            uart_sequence = (uart_sequence + 1) & 0xFF

        # The physical display receives a 640x480, 4:3 annotated frame.
        disp.show(img)

        # Publish the same annotated frame to the browser. Encoding less
        # frequently protects the YOLO/control loop from JPEG overhead.
        if STREAM_ENABLED:
            stream_frame_counter += 1

            if stream_frame_counter >= STREAM_EVERY_N_FRAMES:
                stream_frame_counter = 0
                publish_stream_frame(img)
finally:
    stop_stream_server()
