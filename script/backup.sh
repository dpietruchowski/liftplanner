#!/bin/bash

PACKAGE_NAME="com.liftplanner.app"
DB_NAME="liftplanner.db"
BACKUP_DIR="$HOME/liftplanner_backup"

mkdir -p "$BACKUP_DIR"

if [ "$1" == "restore" ]; then
    adb push "$BACKUP_DIR/$DB_NAME" /sdcard/$DB_NAME
    adb shell run-as $PACKAGE_NAME cp /sdcard/$DB_NAME /data/data/$PACKAGE_NAME/files/$DB_NAME
else
    adb shell run-as $PACKAGE_NAME cp /data/data/$PACKAGE_NAME/files/$DB_NAME /sdcard/$DB_NAME
    adb pull /sdcard/$DB_NAME "$BACKUP_DIR/$DB_NAME"
fi
