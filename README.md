[![Build status](https://ci.appveyor.com/api/projects/status/o5tgk5n932v448vb?svg=true)](https://ci.appveyor.com/project/Wohlstand/itemscene)

# ItemScene
A small custom implementation of item scene to replace heavy and unoptimized QGraphicsScene in the PGE Editor

# Compare to QGraphicsScene:
* Fast feed-back and quick animation processing with no matter of number of elements (QGraphicsScene lags when 20000~40000 elements are stored in a scene. New class doesn't lags while over 1000000 elements are placed on the scene!)
* Correct processing of context menu on any operating system without of any bugs
* Ability to globally disable an element dragging while processing a "placing" / "erasing" / "drag-scrolling" modes
* Accurate and quick camera positioing with convenient and flexible controlling way, and without of any workarounds
* Support for multi-threading while building in-scene data (therefore is possible to draw the "loading" dialog without of spawning of extra dialog boxes, and without of freezing of entire application)
* Better processing of a zoom anchor (unlike of QGraphicsScene, the mouse position anchor will work while mouse is inside of viewport, and will work from center of viewport while mouse is out of viewport (off-screen))

# Download
[Working demo for Win32 can be got here](http://wohlsoft.ru/docs/_laboratory/_Builds/win32/item-scene/item-scene-demo-win32.zip)

# Controlling
* Arrow keys (Scroll camera left-right-up-down)
* Ctrl+left mouse click - toggle selection of a single element
* Shift+left mouse hold and move - additive rectangular selection (without of elements dragging)
* Ctrl+Shift + left mouse hold and move - reversive rectangular selection
* Mouse wheel - scroll vertically
* Ctrl + Mouse wheel - scroll horizontally
* Alt + Mouse wheel - zoom in/out

# Building from sources
```bash
git clone https://github.com/WohlSoft/ItemScene.git
```
* This demo can be built on almost any platform which is supported by Qt (Linux, Windows, Mac OS X).
* You are required to have [latest Qt 5 package which you can take here](https://www.qt.io/download-open-source/)
Simply open the ItemScene.pro in the Qt Creator and run the build of project.

