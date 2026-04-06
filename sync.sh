#!/bin/bash

# الانتقال للمجلد
cd ~/Smart-Hive || exit

# جلب التحديثات ودمجها صامتاً
git pull --rebase origin main

if [ $? -eq 0 ]; then
    echo "[OK] Updated to latest version."
else
    echo "[ERROR] Sync failed. Check local changes."
fi
