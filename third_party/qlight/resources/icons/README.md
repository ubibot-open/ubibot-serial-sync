# Built-in icons

The SVG files in this directory are a subset of **Fluent UI System Icons** by Microsoft Corporation, released under the MIT License (see [LICENSE-fluentui-system-icons.txt](LICENSE-fluentui-system-icons.txt)).

- Source: <https://github.com/microsoft/fluentui-system-icons>
- Pinned commit: `a563cf9166f4f91aa617557ed272612b7f0a2f72`
- Style/size used: `assets/<Name>/SVG/ic_fluent_<name>_20_regular.svg`, saved here as `<name>.svg`
- Files are unmodified. Their fill colour is ignored at runtime: QLight uses each SVG as an alpha mask and tints it.

They are embedded into the library as Qt resources under `:/qlight/icons/`. The copyright and licence notice above must accompany binaries that include them.

To add an icon: fetch the same-named file from the pinned commit, add its name to `Icon` in `include/QLight/FluentIcon.h` and to the name table in `src/FluentIcon.cpp` (a `static_assert` keeps the two in sync).

"Fluent UI System Icons" is a Microsoft project; this repository is not affiliated with or endorsed by Microsoft.
