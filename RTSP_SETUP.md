# RTSP 재생 설정 (UE 5.6, VlcMedia)

이 문서는 본 프로젝트에서 RTSP 스트림이 정상 재생되도록 구성한 내용을 정리한 것입니다.

## 원본 플러그인 저장소

- VlcMedia 원본: `https://github.com/ue4plugins/VlcMedia`
- 본 프로젝트에는 위 플러그인을 프로젝트 플러그인 형태로 포함해 커스터마이징했습니다.

## 환경

- 프로젝트 경로(예시): `<YourProject>/Plugins/VlcMedia`
- 엔진: Unreal Engine 5.6.x
- RTSP 예시: `rtsp://210.99.70.120:1935/live/cctv001.stream`
  - 테스트 주소 출처: [충청남도 천안시_교통정보 CCTV](https://www.data.go.kr/data/15063717/fileData.do)

## 필요한 것 (사전 설치/준비)

- Unreal Engine 5.6.x
- C++ 빌드를 위한 Visual Studio (MSVC 툴체인)
- 별도의 VLC 설치는 필요 없음  
  - LibVLC 바이너리를 플러그인 폴더에 포함해서 사용합니다.

## LibVLC 구성

- LibVLC 위치:
  - `Plugins/VlcMedia-master/ThirdParty/vlc/Win64`
- 사용 버전:
  - LibVLC 3.0.20 (Win64 zip 기반)
- vmem 출력 강제 및 HW 디코더 비활성화 상태로 동작

## 액터 구성 (CCTVStreamActor)

### 에디터에서 만든 MediaPlayer/MediaTexture 사용 (권장)

런타임 생성 대신 에디터에서 만든 에셋을 연결하면 일관성이 좋아집니다.

1) **Media Player** 에셋 생성  
2) 생성 시 **Video Output Media Texture** 체크 → Media Texture 자동 생성  
3) 액터에 다음 에셋을 지정  
   - `MediaPlayerAsset`  
   - `MediaTextureAsset`  
4) `BaseMaterial`에는 `Texture` 파라미터가 있는 머티리얼을 지정

### 주요 프로퍼티

- `RtspUrl`: 재생할 RTSP URL
- `OverridePlayerName`: 기본값 `VlcMedia`
- `bUseTCP`: true이면 `?transport=tcp`를 붙여 TCP 강제

## 성능 최적화

- **복수 스트림**: 최대 8개 동시 재생 권장 (메모리 제한)
- **해상도**: 1080p 내에서 조정 (4K는 메모리 과다 사용)
- **재생 관리**: 사용하지 않는 스트림은 Pause 상태로 전환
- **프레임 버퍼**: 필요시 30fps로 제한하여 부하 감소

## 보안 고려사항

- **인증 URL**: `rtsp://<user>:<password>@<host>:<port>/<path>` (실계정/실비밀번호 직접 노출 금지)
- **네트워크**: 방화벽에서 554(RTSP), 1935(RTMP) 포트 허용
- **TLS**: 프로덕션에서는 `rtsps://` 사용 고려

## 디버깅 명령어

### UE5 콘솔 변수
```bash
vt.VlcMedia.LogLevel 4
vt.VlcMedia.ShowStats 1
Media.LogLevel Verbose
Media.Player.ShowTime 1
```

### 스트림 상태 확인
```cpp
// CCTVStreamActor에서
bool IsStreamHealthy() const;
float GetCurrentBitrate() const;
```

## 배포 테스트 케이스

| 시나리오 | 검증 항목 | 통과 기준 |
| --- | --- | --- |
| 단일 스트림 24시간 | 안정성 | 크래시 없음 |
| 다중 스트림 (4개) | 성능 | 메모리 < 2GB |
| 네트워크 장애 | 복구력 | 5초 내 재연결 |

## 에러 코드별 대응표

| 증상 | 원인 | 해결책 |
| --- | --- | --- |
| 검은 화면 | vmem 실패 | `--avcodec-hw=none` |
| 연결 타임아웃 | 방화벽 | `?transport=tcp` |
| 소리만 나옴 | 비디오 출력 | vmem 설정 확인 |

## 트러블슈팅

### 미디어 플레이어는 재생되는데 머티리얼이 검정일 때

1) `MediaTexture`가 올바른 `MediaPlayer`에 연결되어 있는지 확인  
2) 머티리얼 파라미터 이름이 `Texture`인지 확인  
3) 로그에서 `StaticVideoDisplayCallback` 호출 여부 확인
