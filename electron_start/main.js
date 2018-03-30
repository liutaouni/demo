const electron = require('electron')
// Module to control application life.
const app = electron.app
// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

const path = require('path')
const url = require('url')

//用一个 Tray 来表示一个图标,这个图标处于正在运行的系统的通知区 ，通常被添加到一个 context menu 上.
const Menu = electron.Menu
const Tray = electron.Tray

//托盘对象
var appTray = null

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow () {
  // Create the browser window.
  mainWindow = new BrowserWindow({width: 800, height: 600, skipTaskbar:false})

  // and load the index.html of the app.
  mainWindow.loadURL(url.format({
    pathname: '/ftask/web/app/login.html',
    protocol: 'http',
    host: 'app01.yugusoft.com',
    slashes: true
  }))

//系统托盘右键菜单
     var trayMenuTemplate = [
         {
             label: '设置',
             click: function () {} //打开相应页面
         },
          {
             label: '反馈',
             click: function () {}
         },
         {
             label: '帮助',
             click: function () {}
         },
         {
             label: '关于',
             click: function () {}
         },
         {
             label: '退出',
             click: function () {
                 //ipc.send('close-main-window');
                 	mainWindow.destroy()
                  app.quit();
             }
         }
     ];
 
     //系统托盘图标目录
     trayIcon = path.join(__dirname, 'tray')
 
     appTray = new Tray(path.join(trayIcon, 'app.ico'))
 
     //图标的上下文菜单
     const contextMenu = Menu.buildFromTemplate(trayMenuTemplate)
 
     //设置此托盘图标的悬停提示内容
     appTray.setToolTip('This is my application.')
 
     //设置此图标的上下文菜单
     appTray.setContextMenu(contextMenu)

     //系统托盘图标闪烁
     var count = 0,timer = null;
     timer=setInterval(function() {
         count++;
         if (count%2 == 0) {
             appTray.setImage(path.join(trayIcon, 'app.ico'))
         } else {
             appTray.setImage(path.join(trayIcon, 'appa.ico'))
         }
     }, 600)

     //单点击 1.主窗口显示隐藏切换 2.清除闪烁
     appTray.on("click", function(){
         if(!!timer){
             appTray.setImage(path.join(trayIcon, 'app.ico'))
             //主窗口显示隐藏切换
             if(mainWindow.isMinimized()){
             		mainWindow.show()
             }else{
             		mainWindow.isVisible() ? mainWindow.hide() : mainWindow.show()
             }
         }
     })

  // Open the DevTools.
  // mainWindow.webContents.openDevTools()

  mainWindow.on('close', function (e) {
  	e.preventDefault()
		mainWindow.hide()
  })

  // Emitted when the window is closed.
  mainWindow.on('closed', function () {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    mainWindow = null
  })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
//app.on('window-all-closed', function () {
//  // On OS X it is common for applications and their menu bar
//  // to stay active until the user quits explicitly with Cmd + Q
//  if (process.platform !== 'darwin') {
//    app.quit()
//  }
//})

app.on('activate', function () {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (mainWindow === null) {
    createWindow()
  }
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
