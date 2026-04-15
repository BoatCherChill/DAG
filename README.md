# Редактор направленных ациклических графов.

## Сборка

### 1. Клонирование

```bash
git clone https://github.com/BoatCherChill/DAG.git
cd DAG
```

### 2. Настройка пути к Qt

Откройте `CMakeLists.txt` и укажите:

```cmake
set(Qt5_DIR "C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")
```

### 3. Сборка

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 4. Подготовка к запуску

```bash
cd Release

copy C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\Qt5Core.dll .
copy C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\Qt5Widgets.dll .
copy C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin\Qt5Gui.dll .

mkdir platforms
copy C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\plugins\platforms\qwindows.dll platforms\
```

### 5. Запуск

```bash
DAG.exe
```
