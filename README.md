# seeso4keyboard

基于 VisualCamp SeeSo Windows C++ sample 的视线采集实验，使用 OpenCV 摄像头、SeeSo 校准和 CSV 记录。上游参考：[seeso-sample-windows](https://github.com/visualcamp/seeso-sample-windows)。

[在线文档](https://xiaoshecode.github.io/xiaoshecode/repos/seeso4keyboard/)

## 结构

| 文件 | 职责 |
| --- | --- |
| `main.cpp` | 初始化、环境变量读取、校准数据加载、事件连接和窗口循环 |
| `camera_thread.*` | 摄像头采集线程及本地 AVI 记录 |
| `tracker_manager.*` | SeeSo 回调、校准状态和视线坐标 |
| `view.*`、`drawables.h` | OpenCV GUI |
| `priority_mutex.*`、`simple_signal.h` | 线程同步与事件连接 |
| `utils/CSVLogger.hpp` | CSV 转义、写入和文件生命周期 |
| `tests/support_test.cpp` | 不依赖 SDK 的回归测试 |
| `opencv/`、`seeso/` | 用户本地提供的依赖，不纳入仓库 |

## 环境与构建

需要 Windows、C++11 编译器、CMake 3.5+、兼容的 OpenCV/SeeSo Windows SDK 和 SeeSo 授权。SDK 获取入口：[SeeSo Console](https://manage.seeso.io/)。这些商业 SDK 未随仓库分发。

当前 CMake 使用 `add_subdirectory(opencv)` 和 `add_subdirectory(seeso)`。因此两个目录必须提供各自的 **CMakeLists.txt 包装文件**，分别导出名为 `opencv`、`seeso` 的 target（含 include 和链接信息），以及运行时 DLL 路径 `OPENCV_DLL`、`SEESO_DLL`。仅复制官方二进制压缩包不保证具备这些包装文件。缺少时配置会明确报错，完整 SDK 集成仍需在本机完成。

```powershell
cmake -S . -B build/app
cmake --build build/app --config Release
$env:SEESO_LICENSE_KEY = '由 SeeSo 签发的本机密钥'
.\build\app\Release\seeso_windows.exe
```

密钥从环境变量读取，**不要写回源码或提交到 Git**。旧提交曾包含开发密钥，本次仅移除当前代码中的值；作者仍需在供应商控制台撤销/轮换旧密钥，历史记录未改写。

## 使用

默认摄像头索引 0；按 `C` 开始五点校准，`Esc` 退出。校准数据保存到运行目录 `calibration_data.txt`，视线输出到 `gaze.csv`（每次启动重新写入），视频保存为 `recording-时间戳.avi`。运行目录需要写权限。校准文件不存在时进入未校准状态；格式错误时忽略旧文件并提示重新校准。

## 独立验证

无需 SeeSo/OpenCV SDK 即可测试同步和 CSV 支撑组件：

```sh
cmake -S . -B build/support -DSEESO_SUPPORT_TESTS_ONLY=ON
cmake --build build/support --config Release
ctest --test-dir build/support -C Release --output-on-failure
```

2026-10-30：MSVC 19.51 + CMake 构建通过，CTest 1/1 通过，测试中的断言在 Release 下仍开启。覆盖高优先级 try-lock 失败后低优先级可继续获取锁、信号断开、CSV 引号/逗号转义。

已修复初始化参数缺失、缺失 CSVLogger 实现、重复校准被状态标记阻挡、相机 writer 未初始化/释放顺序，以及硬编码个人视频路径。完整程序未编译或运行：本环境没有 SeeSo SDK、OpenCV C++ 包装和授权；摄像头、SDK 回调线程行为和 GUI 集成尚未验证。

新增代码遵循 `.editorconfig`，输出/校准/本地 SDK 已在 `.gitignore` 中排除。保留上游来源；SDK 和示例再分发分别受其授权约束。
