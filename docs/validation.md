# 验证与发布检查

## 自动化

- `xword_test`：原有基础生成、模板文本／块／图片回归。
- `xword_test_eq`：公式转换回归。
- `xword_test_v2`：组合功能、分节边界、前向引用、资源、错误和重复保存；生成 `v2_*.docx` 验证工件。
- `xword_test_regressions`：中文路径、不同 XML 前缀、跨 run 变量、范围顺序、失败保护、同名图片及 SVG 回退。
- `xword_test_python`：绑定行为、父对象生命周期、C++／Python 逐部件规范化 XML 一致性。启用 `XWORD_BUILD_PYTHON` 时注册。

所有检查在 Release 中也执行，不依赖被 NDEBUG 移除的 assert；检查失败直接返回失败状态，不打开 CRT 弹窗。测试在各自构建目录输出，CTest 设置超时。

仓库的 `build-and-validate` 工作流在 Windows／Linux 构建 C++ 与 Python、运行测试、执行官方 schema 验证并构建 wheel。工作流文件的存在不代表远端 CI 已执行，发布时应检查实际运行结果。

```powershell
ctest --test-dir build/release -C Release --output-on-failure
dotnet run --project tools/OpenXmlValidator -- build/release/v2_features.docx build/release/v2_template_out.docx
```

官方校验工具固定使用 Open XML SDK 3.0.1、Office2019 验证目标，报告部件 URI、XPath 和错误。默认 .NET 8。环境仅有旧 SDK 时可显式覆盖 TargetFramework 以辅助检查，但发布流水线应使用受支持的 .NET 8 SDK。

Release、Debug 和 v141 均需检查 C++ 生成。Python 扩展必须用当前解释器的头文件和库构建，并对安装到独立前缀后的包再次执行 `test_python.py <prefix>`。更换 Python 时使用全新构建目录。

## Word 客户端验收

使用 `xword_demo_v2 <output.docx> [image.png]` 生成综合示例，并在 Word 2019、2021 或 Microsoft 365 中检查：

1. 打开无修复提示，正文、图片、表格、公式及批注均可见。
2. 首页／偶数页／默认页眉页脚与继承、空白覆盖符合配置；横竖版、分栏、五种分节及页码重启正确。
3. 更新目录及所有域后，图表编号、目录和交叉引用正确；点击链接跳到目标位置。
4. 脚注尾注的位置、格式和节内重启正确；表格表头重复，禁止拆行的行保持完整。
5. Word“管理源”中出现文档来源；引用和参考文献列表可更新、编辑及切换内置样式。
6. 检查文件属性与自定义属性，批注作者和日期正确。

WPS／LibreOffice 仅做打开、基础内容和排版冒烟检查，不要求 Word 原生文献域管理能力一致。

Schema 校验不等同于视觉排版验收。无 Office 客户端的自动化环境不宣称已验证页面总数、最终域刷新、来源管理 UI 或像素级一致性。

## 本次本地验证记录

- MSVC 19.44 Debug、MSVC v141 Release：四项 C++ 测试通过。
- MSVC 19.44 Release + CPython 3.12：五项 C++／Python 测试通过，包括生成包一致性和生命周期。
- 代表性功能、图片、模板、页眉页脚、空文档、样式默认值及继承回归、跨 run 模板替换和 Word 另存文档经 Open XML SDK 3.0.1 校验，错误数为 0。
- 本机实际客户端是 Word 16.0.4266（Office 2016），隐藏实例打开综合示例并刷新域，返回错误码 0；识别 7 节、1 个脚注、1 个尾注、1 条批注、1 个文献来源，另存副本为 9 页。
- Word 副本中的 IEEE 引文显示 `[1]`，参考文献列表产生 Microsoft 来源的格式化条目，未找到引用错误文本。
- 匹配 CPython 3.12 的离线 wheel 已制作、安装到独立目录并通过 Python 与 C++ 包一致性测试。常规 scikit-build-core 源码 wheel 路径交由 CI 验证，本次未安装额外构建依赖。
- 最终样式修正后重新通过三个配置的全部测试，刷新离线 wheel，并核对安装目录、wheel 及 CMake 安装产物的二进制字节一致；安装包导入版本为 2.0.0。

本地 Word 检查不是 Word 2019／2021／Microsoft 365、WPS 和 LibreOffice 的逐版本或人工视觉验收。可用 `tools/validate_word.ps1 -InputPath ... -OutputPath ...` 在相应客户端执行；输出必须为新路径，原文件只读打开。
