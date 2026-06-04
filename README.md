# Raspberry Pi Device Controller — TCP/HTTP Server

[한국어](#한국어) · [English](#english)

---

## English

A multi-threaded device control server running on Raspberry Pi 4, controlling LED, buzzer, CDS light sensor, and 7-segment display through both a terminal CLI and a web browser interface.

### Features

- **TCP-based remote device control** — LED brightness (3 levels), buzzer with Happy Birthday melody, light sensor reading, 7-segment display with countdown
- **Two client types** — terminal CLI on Ubuntu and a web browser UI, both handled by the same server
- **Multi-threaded architecture** — accept loop spawns one detached thread per connection; persistent device threads handle hardware
- **Pluggable device control** — all device logic in `libcontrol.so` loaded via `dlopen`; library can be replaced without rebuilding the server
- **Daemonized server** — runs in background, logs to `server.log`

### Architecture

```
                  ┌────────────────────────────────────┐
                  │  Server (Raspberry Pi 4)           │
                  │                                    │
  ┌────────────┐  │  ┌──────────────┐                  │
  │  Terminal  │──┼─►│ accept loop  │                  │
  │  client    │  │  │   spawns     │                  │
  └────────────┘  │  │client_thread │  ┌─────────────┐ │
                  │  └──────┬───────┘  │ Device      │ │
  ┌────────────┐  │         │          │ threads     │ │
  │  Browser   │──┼─────────┘          │  led        │ │
  │            │  │                    │  buzzer     │ │
  └────────────┘  │                    │  seg        │ │
                  │                    │  cds        │ │
                  │                    │  auto_light │ │
                  │                    └──────┬──────┘ │
                  │                           │        │
                  │                           ▼        │
                  │                  ┌──────────────┐  │
                  │                  │libcontrol.so │  │
                  │                  └──────┬───────┘  │
                  └─────────────────────────┼──────────┘
                                            │
                                       GPIO / I2C
                                            │
                          ┌─────────────────┼────────────────┐
                          │                 │                │
                         LED              Buzzer            CDS
                                                          7-Segment
```

### Hardware Setup

| Device | Pin (WiringPi) | Notes |
|---|---|---|
| LED | 1 | softPwm, range 100 |
| Buzzer | 6 | softTone, passive buzzer |
| CDS sensor | I2C 0x48 | analog, returns 0-255 |
| 7-Segment a-g | 27, 28, 29, 25, 24, 23, 22 | common anode (ON = LOW) |

### Build

Requires `aarch64-linux-gnu-gcc` for cross-compilation and `wiringPi` headers for the Pi.

```bash
git clone git@github.com:sinzzangu/VEDA_TCP_RASSPI_TRANSMISSION.git
cd VEDA_TCP_RASSPI_TRANSMISSION
make             # builds libcontrol, server, client
```

Edit `Makefile` to set your Pi's SSH address before deploying:

```makefile
PI_ADDR = juanb0510@100.72.6.87
PI_DIR  = ~/raspi_project/
```

Deploy to Pi:

```bash
make send        # scp server + libcontrol.so + index.html + server.conf
```

### Run

On the Pi:

```bash
cd ~/raspi_project
./server         # daemonizes, prints PID, logs to server.log
tail -f server.log
```

To stop the daemon:

```bash
pkill -x server
```

### Usage

**Terminal client (from Ubuntu):**

```bash
./client/client
# select from hierarchical menu — LED / BUZZER / SEG / CDS
```

**Web client:**

Open `http://<pi-ip>:8080` in any browser. Card-based UI with buttons for each device. Response log shows server replies in real-time.

### Protocol

Plain-text commands over TCP (newline-terminated). HTTP requests are also accepted on the same port — the server detects HTTP by the request prefix and parses the command from the POST body.

| Command | Description |
|---|---|
| `LED ON` / `LED OFF` | Turn LED on / off |
| `LED BRIGHT <0\|1\|2>` | Set brightness level |
| `BUZZER ON` / `BUZZER OFF` | Play / stop Happy Birthday |
| `SEG DISPLAY <0-9>` | Show digit on 7-segment |
| `SEG CLEAR` | Clear 7-segment |
| `SEG COUNTDOWN <0-9>` | Countdown with buzzer at 0 |
| `CDS READ` | Read raw sensor value |
| `CDS AUTO` | Auto-control LED based on threshold |

Responses use HTTP-like status lines: `200 OK`, `503 BUSY`, `400 BAD REQUEST`.

### Directory Structure

```
.
├── Makefile               # top-level build (libcontrol, server, client, deployment)
├── libcontrol/            # shared device library (libcontrol.so)
│   ├── control.c / .h
│   ├── led.c / buzzer.c / cds.c / num_display.c
├── server/                # TCP/HTTP server
│   ├── server.c           # main + accept loop
│   ├── daemon.c           # double-fork daemonization
│   ├── client_handler.c   # per-connection thread
│   ├── setup.c            # dlopen + device init + socket
│   ├── config.c           # server.conf parser
│   ├── protocol.c         # command parser
│   ├── devices.c          # device thread dispatch + HTTP fd registry
│   ├── http.c             # HTTP request parsing + HTML serving
│   ├── led_thread.c       # device threads (5 total)
│   ├── buzzer_thread.c
│   ├── seg_thread.c
│   ├── cds_thread.c
│   ├── auto_light_thread.c
│   ├── index.html         # web UI
│   └── server.conf        # port + threshold config
└── client/
    └── client.c           # terminal client with hierarchical menu
```

---

## 한국어

라즈베리파이 4에서 동작하는 멀티스레드 장치 제어 서버. LED, 부저, CDS 조도센서, 7세그먼트를 터미널 CLI와 웹 브라우저 양쪽에서 제어할 수 있다.

### 주요 기능

- **TCP 기반 원격 장치 제어** — LED 밝기 3단계, Happy Birthday 노래 재생 부저, 조도센서 읽기, 7세그 카운트다운
- **두 종류의 클라이언트** — Ubuntu 터미널 CLI와 웹 브라우저 UI. 같은 서버가 양쪽 모두 처리
- **멀티스레드 구조** — `accept` 루프가 연결마다 detached 스레드 생성, 장치 스레드는 서버 기동 시 상시 동작
- **플러그인형 장치 제어** — 모든 장치 로직이 `libcontrol.so`에 있으며 `dlopen`으로 로드. 서버 재빌드 없이 라이브러리 교체 가능
- **데몬화된 서버** — 백그라운드 실행, `server.log`에 로그 기록

### 시스템 구조

```
                  ┌────────────────────────────────────┐
                  │  서버 (Raspberry Pi 4)              │
                  │                                    │
  ┌────────────┐  │  ┌──────────────┐                  │
  │  터미널      │──┼─►│ accept 루프  │                  │
  │  클라이언트   │  │  │ client_thread│                  │
  └────────────┘  │  │   생성       │  ┌─────────────┐ │
                  │  └──────┬───────┘  │ 장치 스레드   │ │
  ┌────────────┐  │         │          │  led        │ │
  │  브라우저    │──┼─────────┘          │  buzzer     │ │
  │            │  │                    │  seg        │ │
  └────────────┘  │                    │  cds        │ │
                  │                    │  auto_light │ │
                  │                    └──────┬──────┘ │
                  │                           │        │
                  │                           ▼        │
                  │                  ┌──────────────┐  │
                  │                  │libcontrol.so │  │
                  │                  └──────┬───────┘  │
                  └─────────────────────────┼──────────┘
                                            │
                                       GPIO / I2C
                                            │
                          ┌─────────────────┼────────────────┐
                          │                 │                │
                         LED              부저              CDS
                                                          7세그먼트
```

### 하드웨어 배선

| 장치 | 핀 (WiringPi) | 비고 |
|---|---|---|
| LED | 1 | softPwm, range 100 |
| 부저 | 6 | softTone, passive buzzer |
| 조도센서 | I2C 0x48 | 아날로그, 0-255 반환 |
| 7세그 a-g | 27, 28, 29, 25, 24, 23, 22 | common anode (ON = LOW) |

### 빌드

크로스 컴파일을 위해 `aarch64-linux-gnu-gcc`와 Pi용 `wiringPi` 헤더가 필요하다.

```bash
git clone git@github.com:sinzzangu/VEDA_TCP_RASSPI_TRANSMISSION.git
cd VEDA_TCP_RASSPI_TRANSMISSION
make             # libcontrol, server, client 빌드
```

배포 전에 `Makefile`에서 Pi의 SSH 주소를 설정:

```makefile
PI_ADDR = juanb0510@100.72.6.87
PI_DIR  = ~/raspi_project/
```

Pi로 전송:

```bash
make send        # server + libcontrol.so + index.html + server.conf을 scp
```

### 실행

Pi에서:

```bash
cd ~/raspi_project
./server         # 데몬화, PID 출력, server.log에 로그 기록
tail -f server.log
```

데몬 종료:

```bash
pkill -x server
```

### 사용 방법

**터미널 클라이언트 (Ubuntu에서):**

```bash
./client/client
# 계층적 메뉴에서 선택 — LED / BUZZER / SEG / CDS
```

**웹 클라이언트:**

브라우저에서 `http://<pi-ip>:8080` 접속. 카드 기반 UI로 각 장치 제어. 응답 로그에서 서버 응답을 실시간 확인.

### 프로토콜

줄바꿈 단위 평문 명령을 TCP로 전송. 같은 포트에서 HTTP 요청도 받으며, 서버가 요청 시작 부분으로 HTTP 여부를 판단해 POST 본문에서 명령을 추출한다.

| 명령 | 설명 |
|---|---|
| `LED ON` / `LED OFF` | LED 켜기 / 끄기 |
| `LED BRIGHT <0\|1\|2>` | 밝기 단계 설정 |
| `BUZZER ON` / `BUZZER OFF` | Happy Birthday 재생 / 정지 |
| `SEG DISPLAY <0-9>` | 7세그에 숫자 표시 |
| `SEG CLEAR` | 7세그 끄기 |
| `SEG COUNTDOWN <0-9>` | 카운트다운 + 0에서 부저 |
| `CDS READ` | 센서 원시값 읽기 |
| `CDS AUTO` | 임계값 기반 LED 자동 제어 |

응답은 HTTP-like 상태 라인을 사용: `200 OK`, `503 BUSY`, `400 BAD REQUEST`.

### 디렉터리 구조

```
.
├── Makefile               # 최상위 빌드 (libcontrol, server, client, 배포)
├── libcontrol/            # 공유 장치 라이브러리 (libcontrol.so)
│   ├── control.c / .h
│   ├── led.c / buzzer.c / cds.c / num_display.c
├── server/                # TCP/HTTP 서버
│   ├── server.c           # main + accept 루프
│   ├── daemon.c           # double-fork 데몬화
│   ├── client_handler.c   # 연결별 스레드
│   ├── setup.c            # dlopen + 장치 초기화 + 소켓
│   ├── config.c           # server.conf 파서
│   ├── protocol.c         # 명령 파서
│   ├── devices.c          # 장치 스레드 디스패치 + HTTP fd 레지스트리
│   ├── http.c             # HTTP 파싱 + HTML 제공
│   ├── led_thread.c       # 장치 스레드 (총 5개)
│   ├── buzzer_thread.c
│   ├── seg_thread.c
│   ├── cds_thread.c
│   ├── auto_light_thread.c
│   ├── index.html         # 웹 UI
│   └── server.conf        # 포트 + 임계값 설정
└── client/
    └── client.c           # 계층적 메뉴 터미널 클라이언트
```