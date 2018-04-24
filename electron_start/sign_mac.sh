#!/bin/bash
  
  # 你的应用名称
  APP="YUGU"
  # 要签名的应用路径
  APP_PATH="Fishbone_out/$APP-mas-x64/$APP.app"
  # 生成安装包路径
  RESULT_PATH="Fishbone_out/$APP-mas-x64/$APP.pkg"
  # 开发者应用签名证书
  APP_KEY="3rd Party Mac Developer Application: Yugusoft (Beijing) Technology Co., Ltd. (JGBX7E5U56)"
  INSTALLER_KEY="3rd Party Mac Developer Installer: Yugusoft (Beijing) Technology Co., Ltd. (JGBX7E5U56)"
  # 授权文件路径
  CHILD_PLIST="child.plist"
  PARENT_PLIST="parent.plist"
  LOGINHELPER_PLIST="loginhelper.plist"
  # 截图程序路径
  SCRENN_CAPTURE_PATH="Fishbone_out/ScreenCapture.app"

  FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"
  
  cp -fR $SCRENN_CAPTURE_PATH $FRAMEWORKS_PATH

  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/Electron Framework.framework/Versions/A/Electron Framework"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/Electron Framework.framework/Versions/A/Libraries/libffmpeg.dylib"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/Electron Framework.framework/Versions/A/Libraries/libnode.dylib"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/Electron Framework.framework"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper.app/Contents/MacOS/$APP Helper"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper.app/"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper EH.app/Contents/MacOS/$APP Helper EH"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper EH.app/"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper NP.app/Contents/MacOS/$APP Helper NP"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$FRAMEWORKS_PATH/$APP Helper NP.app/"
  codesign -s "$APP_KEY" -f --entitlements "$CHILD_PLIST" "$APP_PATH/Contents/MacOS/$APP"
  codesign -s "$APP_KEY" -f --entitlements "$PARENT_PLIST" "$APP_PATH"
  
  productbuild --component "$APP_PATH" /Applications --sign "$INSTALLER_KEY" "$RESULT_PATH"