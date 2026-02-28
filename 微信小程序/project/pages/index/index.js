// index.js
Page({
  // 页面初始数据
  data: {
    Temp: 0, // 温度
    Hum: 0, // 湿度
    Qiya: 0, // 气压
    LED: '', // LED状态
    loading: false // 加载状态，可选，提升用户体验
  },

  // 配置项（建议抽离，保持代码整洁）
  config: {
    authorization: "version=2018-10-31&res=products%2FnE4HArK3N3%2Fdevices%2FPena&et=1893495429&method=md5&sign=wGvfT7jsJ3%2F4vkjSfjFcRQ%3D%3D", // 鉴权信息
    product_id: "nE4HArK3N3", // 产品ID
    device_name: "Pena", // 设备名称
    getinfo_url: 'https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=nE4HArK3N3&device_name=Pena', //设备属性最新数据查询地址
    setinfo_url:'https://iot-api.heclouds.com/thingmodel/set-device-property'//设置设备属性
  },

  // 获取OneNET设备数据（优化版）
  Onenet_GetInfo() {
    // 1. 加载状态（可选，让用户知道正在请求）
    this.setData({ loading: true });

    wx.request({
      url: this.config.getinfo_url,
      header: {
        'authorization': this.config.authorization,
        'content-type': 'application/json' // 显式声明请求格式，避免兼容问题
      },
      method: "GET",
      // 成功回调
      success: (e) => {
        console.log("OneNET请求成功返回：", e);
        // 关键优化：数据判空，避免正式版因数据格式异常导致报错
        if (e.data && e.data.data && e.data.data.length >= 4) {
          this.setData({
            Temp: e.data.data[3].value || '未知',
            Hum: e.data.data[2].value || '未知',
            Qiya: e.data.data[1].value || '未知',
            LED: e.data.data[0].value || '未知'
          });
        } else {
          wx.showToast({
            title: '数据格式异常',
            icon: 'none'
          });
          console.error("OneNET返回数据格式异常：", e.data);
        }
      },
      // 失败回调（核心补充：正式版必须加，否则失败无提示）
      fail: (err) => {
        wx.showToast({
          title: '获取数据失败',
          icon: 'none'
        });
        console.error("OneNET请求失败：", err);
        // 常见失败原因排查提示
        if (err.errMsg.includes('request:fail')) {
          console.warn("可能原因：1. 合法域名未配置 2. HTTPS证书错误 3. 网络问题 4. OneNET鉴权过期");
        }
      },
      // 完成回调（无论成功失败，关闭加载状态）
      complete: () => {
        this.setData({ loading: false });
      }
    })
  },
  Onenet_SetAlarmInfo(event)
  {
    const is_checked = event.detail.value; // 获取开关状态
    wx.showToast({
      title: '操作成功', // 提示的文字内容
      icon: 'success', // 图标类型，使用成功图标
      duration: 1000 // 提示框自动隐藏的时间，单位是毫秒
    }), 
    wx.request({
      url: this.config.setinfo_url,
      header:{
        'authorization':this.config.authorization
      },
      method:"POST",
      data: {
        "product_id": this.config.product_id,
        "device_name": this.config.device_name,
        "params": {
          "Led": is_checked
        }     
      },            
    })
  }, 
  onInputChange(e) {
    const inputValue = e.detail.value;
    this.setData({
      verifyCode: inputValue  // 实时更新输入内容
    });

    // 核心逻辑：输入内容严格等于"123"时，立即自动跳转
    if (inputValue === '736') {
      wx.navigateTo({
        url: '/pages/profile/profile' // 跳转至个人简介页
      });
      // 跳转后清空输入框，避免再次进入页面仍显示123
      this.setData({
        verifyCode: ''
      });
    }
    // 输入其他内容（如1、12、124、456等），无任何反应
  },
  checkInputAndJump() {
    // 仅当输入内容为"123"时跳转
    if (this.data.verifyCode === '736') {
      wx.navigateTo({
        url: '/pages/profile/profile'
      });
      // 清空输入框（可选，提升体验）
      this.setData({
        verifyCode: ''
      });
    }
    // 输入错误则无任何反应（不提示、不跳转）
  },
  gotoProfilePage() {
    wx.navigateTo({
      url: '/pages/profile/profile' // 确保路径与新建页面一致
    })
  },
  onLoad() {
    // 实时更新时间
    setInterval(this.Onenet_GetInfo,5000)
  }
  // shi_jian(){
  //   console.log(this.data.Temp)
  //   console.log(this.data.Hum)
  //   this.setData({ //实时页面数据更新
  //     Temp:12
  //   })
  // }
})
