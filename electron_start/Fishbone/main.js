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

const os = require('os')

// 是否正在退出
var willQuit = false

//托盘对象
var appTray = null

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow () {
  platform = os.platform();

  if (platform === 'mas' || platform === 'darwin') {
    var template = [{
      label: "Application",
      submenu: [
          { label: "About Application", selector: "orderFrontStandardAboutPanel:" },
          { type: "separator" },
          { label: "Quit", accelerator: "Command+Q", click: function() { app.quit(); }}
      ]}, {
      label: "Edit",
      submenu: [
          { label: "Undo", accelerator: "CommandOrControl+Z", selector: "undo:" },
          { label: "Redo", accelerator: "Shift+CommandOrControl+Z", selector: "redo:" },
          { type: "separator" },
          { label: "Cut", accelerator: "CommandOrControl+X", selector: "cut:" },
          { label: "Copy", accelerator: "CommandOrControl+C", selector: "copy:" },
          { label: "Paste", accelerator: "CommandOrControl+V", selector: "paste:" },
          { label: "Select All", accelerator: "CommandOrControl+A", selector: "selectAll:" }
      ]}
    ];
    Menu.setApplicationMenu(Menu.buildFromTemplate(template));
  }

  // Create the browser window.
  mainWindow = new BrowserWindow({width: 1000, height: 700})
	
  // and load the index.html of the app.
  mainWindow.loadURL('http://app01.yugusoft.com/ftask/web/index.html#/login?plat=electron')

  //系统托盘图标目录
  trayIcon = path.join(__dirname, 'tray')

  if (platform === 'win32') {
    appTray = new Tray(path.join(trayIcon, 'app.ico'))
  }else{
    appTray = new Tray(path.join(trayIcon, 'app.png'))
  }

  if (platform === 'win32') {
    //系统托盘右键菜单
    var trayMenuTemplate = [
       {
           label: '显示',
           click: function () {
                mainWindow.show()
           }
       },
       {
           label: '退出',
           click: function () {
                app.quit();
           }
       }
    ];

    //图标的上下文菜单
    const contextMenu = Menu.buildFromTemplate(trayMenuTemplate)

    //设置此图标的上下文菜单
    appTray.setContextMenu(contextMenu)
  }

  //设置此托盘图标的悬停提示内容
  appTray.setToolTip('Fishbone')

  //单点击 主窗口显示隐藏切换
  appTray.on("click", function(){
    if (mainWindow === null) {
      createWindow()
    }else if(mainWindow.isMinimized()){
    	mainWindow.restore()
    }else{
      mainWindow.show()
    }
  })

  // Open the DevTools.
  // mainWindow.webContents.openDevTools()

  // 准备关闭窗口
  mainWindow.on('close', function (e) {
    if(!willQuit) {
      e.preventDefault()
      mainWindow.hide()
    }
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
app.on('ready', function () {
  createWindow()
  if (platform === 'mas' || platform === 'darwin') {
  	mainWindow.maximize()
  }
})

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
  }else{
    mainWindow.show()
  }
})

// 准备退出应用
app.on('before-quit', function() {
    willQuit = true
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
