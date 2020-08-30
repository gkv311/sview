#!/bin/bash

# auxiliary script to copy resources (translation files, shaders) to destination folder
TARGET_OUTPUT_DIR=$1
PROJECT_NAME=$2

mkdir -p "${TARGET_OUTPUT_DIR}lang/English/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/Español/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/русский/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/français/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/Deutsch/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/ChineseS/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/ChineseT/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/Korean/"
mkdir -p "${TARGET_OUTPUT_DIR}lang/Czech/"
cp -f -r lang/english/* "${TARGET_OUTPUT_DIR}lang/English/"
cp -f -r lang/spanish/* "${TARGET_OUTPUT_DIR}lang/Español/"
cp -f -r lang/russian/* "${TARGET_OUTPUT_DIR}lang/русский/"
cp -f -r lang/french/*  "${TARGET_OUTPUT_DIR}lang/français/"
cp -f -r lang/german/*  "${TARGET_OUTPUT_DIR}lang/Deutsch/"
cp -f -r lang/chinese/* "${TARGET_OUTPUT_DIR}lang/ChineseS/"
cp -f -r lang/chineset/* "${TARGET_OUTPUT_DIR}lang/ChineseT/"
cp -f -r lang/korean/*  "${TARGET_OUTPUT_DIR}lang/Korean/"
cp -f -r lang/czech/*   "${TARGET_OUTPUT_DIR}lang/Czech/"

if [ -d "shaders" ]; then
  mkdir -p "${TARGET_OUTPUT_DIR}shaders/${PROJECT_NAME}"
  cp -f -r shaders/* "${TARGET_OUTPUT_DIR}shaders/${PROJECT_NAME}/"
fi

if [ -d "web" ]; then
  mkdir -p "${TARGET_OUTPUT_DIR}web"
  cp -f -r web/* "${TARGET_OUTPUT_DIR}web/"
fi
