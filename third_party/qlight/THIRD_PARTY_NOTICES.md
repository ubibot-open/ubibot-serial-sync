# Third-party notices

QLight Widgets itself is released under the MIT License (see [LICENSE](LICENSE)). It uses or can use the
following third-party components. Binaries that include them must carry their notices.

## Included in the repository

### Fluent UI System Icons (47 SVG files)

- Files: `resources/icons/*.svg`, embedded into the library as Qt resources.
- Source: <https://github.com/microsoft/fluentui-system-icons>, commit `a563cf9166f4f91aa617557ed272612b7f0a2f72`
- Copyright (c) 2020 Microsoft Corporation, MIT License. Full text: [resources/icons/LICENSE-fluentui-system-icons.txt](resources/icons/LICENSE-fluentui-system-icons.txt)
- Details and update instructions: [resources/icons/README.md](resources/icons/README.md)

## Optional, downloaded at configure time (not in the repository)

### QWindowKit 1.5.0 - only with `-DQLIGHT_WITH_WINDOWKIT=ON`

- Source: <https://github.com/stdware/qwindowkit> (tag `1.5.0`), fetched with CMake `FetchContent`
  together with its submodules (`qmsetup`, `syscmdline`) and linked statically into `FramelessWindow`.
- License: Apache License 2.0 (QWindowKit; see the `LICENSE` file in its source tree). Its submodules carry
  their own licenses in their source trees. When you distribute a binary built with this option, include the
  Apache-2.0 license text and QWindowKit's copyright notices.

## Linked at build time (not distributed with the library)

### Qt 6

- QLight Widgets links Qt Core, Gui, Widgets and Svg (Qt Test for the tests).
- Open-source Qt is available under LGPLv3 / GPLv3 (and a commercial license). An application using QLight
  Widgets must satisfy the terms of the Qt license it chooses; for LGPLv3 that means, among other things,
  linking Qt dynamically (or otherwise allowing the Qt libraries to be replaced) and shipping the license notice.
  See <https://www.qt.io/licensing/>.

## Not used

QLight Widgets is an independent implementation. It takes visual inspiration from Microsoft's public Fluent
Design guidelines and from QFluentWidgets, but contains no code or assets from QFluentWidgets.
