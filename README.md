# 人工桌面提取助手 LuCBatch  

使用人工桌面 `NovaDesktop` 时可以打开 `Batch_Main.exe` 进行快速提取  
> The .exe(Windows) File compiled with C++ which can do Batch operation  
> of extract NovaDesktop's dynamic wallpaper and static wallpaper.
#### 壁纸版权所有Copyright：米哈游miHoYo  
#### Author: 890mn/Link_Joestar
#### Version: 1.0 
#### Date: 2023/07/20

___

## 环境要求  
已包含编译文件exe运行即可  
如有编译需求，__C++17__ 及以上  

## 编译
```
> g++ -o Batch_Main Batch_Main.cpp
```

## 运行
```
> ./Batch_Main
```

## 提取助手用法
```
> Do you know the drive letter for the N0vaDesktop folder? (Enter the drive letter or leave empty):
```
输入人工桌面所在盘符，也可以选择不输入直接查找   
> C盘默认扫描位置C:/Program Files/N0vaDesktop 其余位置不予搜索

   
```
> Do you want to start downloading files? (y/n):
```
输入Y/y进入提取模式  

```
> Current Random Seed: 26608
> The Programme is running with dynamic screen-refresh.
> You can leave whenever just by you choose [ctrl-c] or [quit by your like]
```
提示出现后保持程序运行然后随便下载吧  
程序退出条件是Ctrl-C组合键或怎么样都行  
