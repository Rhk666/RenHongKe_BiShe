

# 基于多传感融合的多控运动装置监测系统
<div align="center">
  <img src="https://img.shields.io/github/languages/top/Rhk666/RenHongKe_BiShe?color=blue" alt="语言"/>
  <img src="https://img.shields.io/github/last-commit/Rhk666/RenHongKe_BiShe?color=green" alt="最后更新"/>
</div>


**潍坊学院 任洪轲 | 2026年1月22日**


## 📋 项目概览
本项目是一套**多传感融合+多端交互**的智能运动装置监测控制系统，实现「环境感知-多模式控制-云端数据管理-终端可视化」的全流程闭环，适用于仓库检测、地质环境勘探等探测任务。


## ✨ 核心功能特性
| 模块          | 核心能力                                                                 |
|---------------|--------------------------------------------------------------------------|
| 多传感采集    | 搭载**温湿度(ATH20)、气压(BMP280)、GPS(ATGM336H)、人体传感(LD2410C)、IMU六轴陀螺仪** 传感器，实现环境+定位数据实时采集 |
| 多模式控制    | 支持「2.4G遥控/蓝牙遥控」双硬件控制，内置「循迹/方向环/手动」3种行驶模式       |
| 数据云同步    | 采集数据通过ESP8266上传至**OneNET物联网平台**，支持历史数据存储+阈值告警       |
| 多端交互终端  | 提供「微信小程序/安卓蓝牙APP」双终端，实现远程/本地的状态观测+实时控制         |

### 个人微信
<p align="center">
  <img src="https://raw.githubusercontent.com/Rhk666/RenHongKe_BiShe/master/微信小程序码/个人微信.jpg" width="400"/>
</p>