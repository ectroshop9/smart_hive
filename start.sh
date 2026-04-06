#!/bin/bash
export IDF_PATH="$HOME/esp-idf"
export IDF_TOOLS_PATH="$HOME/.espressif"
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.3_py3.12_env"

source $IDF_PYTHON_ENV_PATH/bin/activate

# البحث الديناميكي عن المترجم (أهم خطوة)
COMPILER_DIR=$(find $IDF_TOOLS_PATH/tools -name "xtensa-esp32s3-elf-gcc" -exec dirname {} \; | head -n 1)
export PATH="$IDF_PYTHON_ENV_PATH/bin:$COMPILER_DIR:$IDF_PATH/tools:$PATH"

export PYTHONPATH="$IDF_PATH/tools:$IDF_PATH/tools/kconfig_new:$IDF_PATH/components/micropython/py:$PYTHONPATH"
export IDF_SKIP_CHECK=1
export IDF_COMPONENT_MANAGER=0

echo "✅ المترجم مفعل من: $COMPILER_DIR"
echo "🚀 السمارت هايف جاهز!"
