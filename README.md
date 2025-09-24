# 인생 360 컷 (Life 360 Cut)

Qt와 OpenCV로 구현한 실시간 파노라마 사진 편집기입니다. Intel AI Edge Academy 7기 2팀이 개발했으며, 웹캠 영상에서 여러 장의 이미지를 촬영해 한 장의 파노라마로 합성하고 필터·프레임까지 입힐 수 있는 올인원 편집 환경을 제공합니다.

## 프로젝트 개요

- **프로그램 목적:** 누구나 간단히 360°에 가까운 풍경을 촬영하고, 얼굴을 인식해 캐릭터 프레임을 합성하거나 감성 필터를 적용할 수 있는 데스크탑 애플리케이션
- **개발 기간 / 소속:** Intel AI Edge Academy 7기 팀 프로젝트
- **핵심 기술:** C++17, Qt Widgets, OpenCV 4.5.4 (영상 처리·머신비전)
- **지원 플랫폼:** Linux/Windows (Qt Creator 또는 qmake 기반 빌드)

## 핵심 기능

- **파노라마 이미지 합성** — `cv::Stitcher`를 활용해 특징점 검출·매칭, 호모그래피 계산, 블렌딩까지 자동으로 수행하여 자연스러운 파노라마를 생성합니다. (`Intel_opencv3team/stitch_images.cpp`)
- **자동 왜곡 보정 및 크롭** — 합성 결과에서 검은 여백이 남지 않도록 가장 큰 유효 영역을 탐색하는 `largestAxisAlignedRect` 알고리즘으로 후처리합니다.
- **실시간 필터** — 뽀샤시(`cv::bilateralFilter` + `cv::addWeighted`), 필름 그레인(`cv::randn`), 세피아(`cv::transform`) 등 다양한 필터를 버튼 한 번으로 적용합니다. (`Intel_opencv3team/mainwindow.cpp`)
- **프레임 합성** — Haar Cascade 기반 얼굴 인식(`cv::CascadeClassifier`)으로 캐릭터 프레임을 자동 덧씌우고, 사용자가 직접 ROI를 선택해 수동 프레임도 적용할 수 있습니다. 전체 액자 테두리 역시 지원합니다.
- **직관적인 GUI** — Qt Widgets 레이아웃과 시그널/슬롯을 활용해 촬영, 편집, 저장까지 한 화면에서 처리할 수 있는 사용 흐름을 구성했습니다.

## 작동 흐름

1. 웹캠 프리뷰를 확인하며 원하는 구도로 이동하면서 촬영 버튼으로 연속 이미지를 확보합니다.
2. `완료` 버튼을 누르면 이미지 스티칭과 자동 크롭이 수행되어 파노라마가 생성됩니다.
3. 생성된 파노라마에 필터·프레임·액자를 자유롭게 적용하거나 수동으로 영역을 지정합니다.
4. 결과물을 이미지 파일로 저장하거나 다시 촬영해 새로운 파노라마를 만들 수 있습니다.

## 기술 스택

| 영역 | 사용 기술 |
| --- | --- |
| UI 프레임워크 | Qt 5/6 Widgets, Qt Designer `.ui` |
| 영상 처리 | OpenCV 4.5.4 (Stitcher, ImgProc, CascadeClassifier) |
| 언어 & 표준 | C++17 |
| 개발 도구 | Qt Creator, CMake/qmake, GCC/Clang |

## 프로젝트 구조

```text
Intel_OpenCV_Project_Team2/
├─ Intel_opencv3team/         # Qt 프로젝트 (코드, 리소스, .pro)
│  ├─ main.cpp
│  ├─ mainwindow.(h|cpp|ui)
│  ├─ stitch_images.(h|cpp)
│  ├─ frame1.png, jjangframe.png 등 프레임 리소스
│  └─ project_summary.md      # 기능 요약 문서
├─ build-Intel_opencv3team-Desktop-Debug/
├─ 인텔_OpenCV_프로젝트_2팀.pdf / .pptx   # 발표 자료
└─ 시연 영상.mp4                      # 앱 사용 데모
```

## 코드 구성 및 주요 로직

- `Intel_opencv3team/main.cpp` — Qt 애플리케이션 진입점으로 `MainWindow`를 생성하고 이벤트 루프를 실행합니다. 프로젝트 전반의 초기화를 담당하며 UI 스레드 기반 렌더링을 보장합니다.
- `Intel_opencv3team/mainwindow.cpp` — 카메라 캡처(`cv::VideoCapture`)부터 시그널/슬롯 연결, 상태바 업데이트 등 대부분의 기능이 구현된 핵심 클래스입니다. 
  - `updateFrame()`에서 주기적으로 웹캠 프레임을 가져와 RGB 변환 후 `QLabel`에 출력합니다.
  - `on_captureButton_clicked()`와 `on_doneButton_clicked()`는 촬영 이미지 버퍼링과 스티칭 호출을 담당하며, 성공 시 `displayImage()`로 UI를 갱신합니다.
  - `applyFaceMask()`는 Haar Cascade로 검출된 얼굴 `cv::Rect`마다 알파 채널을 가진 PNG 프레임을 투영하고, 수동 프레임 모드는 `eventFilter()`에서 마우스 드래그 좌표를 캡처해 동일한 합성 로직을 재사용합니다.
  - 필터 함수(`applyBeautyFilter`, `applyFilmGrainFilter`, `applySepiaFilter`)는 OpenCV 필터 API를 래핑해 버튼 한 번에 시각 효과를 적용합니다.
  - `onAddBorderFrameClicked()`는 `cv::copyMakeBorder`와 `cv::rectangle`로 다중 색상 액자를 그려 완성도를 높입니다.
- `Intel_opencv3team/stitch_images.cpp` — 파노라마 합성 전용 유틸로, `stitchImages()`에서 `cv::Stitcher::create`를 사용해 입력 이미지 벡터를 자동 스티칭합니다. 후처리로 `crop_nonblack()`이 호출돼, 모폴로지 연산과 누적 히스토그램 방식(`largestAxisAlignedRect`)으로 최대 유효 영역을 잘라낸 깨끗한 파노라마를 반환합니다.
- `Intel_opencv3team/mainwindow.ui` — Qt Designer로 작성한 UI 레이아웃으로 버튼 그룹, 이미지 표시 영역, 상태바, 프레임 선택 레이아웃 등을 선언합니다. 런타임에 동적으로 삽입하는 버튼들과 연결되어 사용자 경험을 구성합니다.
- `Intel_opencv3team/stitch_images.h` — 파노라마 합성 관련 함수 시그니처를 노출해 GUI 코드에서 쉽게 호출할 수 있도록 합니다.

## 빌드 & 실행

### 준비물

- Qt 5.15+ 또는 Qt 6.x (Widgets 모듈 포함)
- OpenCV 4.5 이상 (`opencv4` pkg-config 사용)
- C++17을 지원하는 컴파일러 (GCC 9+, Clang 10+ 등)
- 실행 폴더에 `haarcascade_frontalface_default.xml` 배치 (얼굴 검출용)

### Qt Creator

1. `Intel_opencv3team/Intel_opencv3team.pro`를 Qt Creator로 엽니다.
2. 시스템에 설치된 Qt Kit과 OpenCV(pkg-config) 경로를 확인합니다.
3. `Run`을 눌러 빌드 후 실행하면 웹캠 프리뷰가 표시됩니다.

### 터미널 빌드 (qmake)

```bash
cd Intel_opencv3team
qmake Intel_opencv3team.pro
make -j$(nproc)
./Intel_opencv3team   # 실행 전 haarcascade XML 복사 필수
```

Windows 환경에서는 `nmake` 또는 `jom`을 사용할 수 있으며, OpenCV와 Qt 경로를 qmake에서 인식하도록 환경 변수를 설정해야 합니다.

## 사용 방법

- **촬영**: `촬영` 버튼으로 이미지를 수집하고 상태바에서 촬영 횟수를 확인합니다.
- **파노라마 생성**: 최소 2장 이상 촬영 후 `완료`를 눌러 합성을 수행합니다.
- **필터**: `뽀샤시`, `필름 질감`, `세피아` 버튼으로 다양한 무드 연출이 가능합니다.
- **프레임**: 얼굴 인식 기반 캐릭터 프레임 또는 `수동 프레임` 모드로 직접 영역을 선택합니다.
- **액자/저장**: `액자 추가`로 테두리를 덧씌우고 `저장` 버튼으로 결과 이미지를 파일로 내보냅니다.
- **재촬영**: `촬영` 버튼을 다시 누르면 초기 상태로 돌아가 새 프로젝트를 시작할 수 있습니다.

## 데모 & 자료

- `시연 영상.mp4` — 실제 사용 장면을 담은 데모 영상
- `인텔_OpenCV_프로젝트_2팀.pdf` / `.pptx` — 프로젝트 소개 및 기술 발표 자료

## 팀 소개

- 오민지 — 얼굴 인식 및 캐릭터 프레임 제작, UI 디자인
- 황경태 — 파노라마 합성, 자동 크롭 알고리즘, 필터 구현
- 황진영 — Qt GUI 통합, 시연 및 발표 준비

## 향후 계획

- 딥러닝 기반 얼굴 인식으로 다양한 각도의 얼굴을 안정적으로 검출
- 학습 기반 이미지 정렬·움직임 보정 모델을 접목해 스티칭 품질 향상

본 저장소의 README는 발표 자료(PDF/PPT)와 `Intel_opencv3team/project_summary.md`를 참고해 작성되었으며, 핵심 소스 코드의 로직과 구조를 함께 담았습니다. 문의 사항이나 개선 아이디어는 Issue로 남겨주세요.
