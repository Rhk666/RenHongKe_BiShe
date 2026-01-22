Page({
  // 返回主页面
  goback() {
    wx.navigateBack({
      delta: 1 // 返回上一级页面
    })
  }
})