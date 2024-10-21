# 人工桌面提取助手 LuCBatch  

使用人工桌面 `N0vaDesktop` 时可以打开 `Batch_Main.exe` / `LuQT.exe` 进行快速提取  
> The .exe(Windows) File compiled with C++ which can do Batch operation  
> of extract NovaDesktop's dynamic wallpaper and static wallpaper.

壁纸版权所有 Copyright：米哈游miHoYo  

Author: Hinar

Version: 1.0 (New Merge)

Date: 2024/10/21

___
## 演示视频
[提取说明](https://www.bilibili.com/video/BV1nN411e75A/)
 | [(CMD) 提取演示](https://www.bilibili.com/video/BV1CntaeoE93/)
  | [(QT) 提取演示](https://www.bilibili.com/video/BV1CntaeoE93/)

## 使用说明

本助手经过两次迭代存在两个版本的程序，以下说明基于此处说明：

- 基于 `Powershell` 的提取助手
    以下对于这一助手简称 **CMD** 版本
    对于 CMD 版本的提取助手，使用时：
    1. 下载 **Release** 中的 `Batch_Main.exe`
    2. 以 **管理员模式** 运行 [如果不听话直接运行闪退就是权限不够被windows拦截了] 
    3. `Do you know the drive letter for the N0vaDesktop folder? (Enter the drive letter or leave empty):`
    
        输入人工桌面所在盘符，也可以选择不输入直接查找（运行必需以管理员权限打开，不然会闪退）   
    
        > C盘默认扫描位置C:/Program Files/N0vaDesktop 其余位置不予搜索

        同时这一步也可以使用 `test.exe` 直接输入人工桌面地址[如：../N0vaDesktopCache/Game]用来代替机器检索

    4. `Do you want to start downloading files? (y/n):`

        输入Y/y进入提取模式  

    5. `Current Random Seed: 26608`
        `The Programme is running with dynamic screen-refresh.`
        `You can leave whenever just by you choose [ctrl-c] or [quit by your like]`

        提示出现后保持程序运行然后随便下载吧

- 基于 `QT` 的提取助手
    以下对于这一助手简称 **QT** 版本
    对于 QT 版本的提取助手，使用时：
    1. 下载 **Release** 中的 `LuQT-Release.zip` 并解压
    2. 双击打开 `LuQT.exe`
    3. 双击 `1 - Locate Path` 自动扫描目录
    4. 双击 `2 - Listen Start` 开始监听
    5. 保持程序运行然后随便下载吧
    6. 如有疑问可以点击 `EX - Log Print` 查看日志输出

注：以上两个版本的输出目录都在同目录下的 `output_mp4` / `output_png` 下
