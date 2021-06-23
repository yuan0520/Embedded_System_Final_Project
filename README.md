# Embedded System Final Project

## (1) how to setup and run my program

### Define calibration table

首先，我先幫車子建立了 calibration table，因為在這次作業中我的速度全部都是由 `set_speed_by_cm` 這個 function 來控制，這樣可以透過直接量測距離來看看車子要走幾秒，比較好算。

![](https://i.imgur.com/3NdFEgb.png)


## 1. Line detection

首先，我是利用 OpenMV 內建的 regression 方法找到一條直線（助教跟我說的），發現滿好用的，只是環境還是要比較乾淨才能去除雜訊，有了這條線之後，我就直接傳一個 RPC 指令給 mbed，並將他的 `length`、`theta`、`rho` 三個數值來呼叫 RPC function。

### Concept Explanation

在這邊我要解釋一下怎麼利用 `rho` 跟 `theta` 這兩個參數來校正車子，使他跟著線的方向移動。

在 OpenMV 的官網中給出了下面這張圖，第一象限內就是 OpenMV 相機可以拍到的範圍，也就是車子看過去的畫面，左上角是原點。

![](https://i.imgur.com/Z9kQglF.png)

`rho` 代表的就是從原點找一條到那條線的「垂直線」的「長度」，當 `rho` 在第四象限時，他的值會是負的，反之當 `rho` 在第一象限時，他的值會是正的。而 `theta` 代表的是從 `rho` 逆時針轉碰到 x 軸時所經過的角度，所以當 `rho` 在第四象限時， 90 < `theta` < 180，而 `rho` 在第一象限時，0 < `theta` < 90。因此我就可以用 `rho` 跟 `theta` 來判斷車子的位置及該調整的角度。

### Function definition

首先我先定義了 `turn_my_forward` 函式，其實就只是剛剛的
 `turn_my` 函式的往前走版本，其實應該可以透過傳入相反的 speed 值就好，不用再定義一個。

![](https://i.imgur.com/gEOPzDa.png)

接下來我定義了 `RPC_go_line` 這個 RPC function，所有動作都會在這裡完成，上面是吃進參數的部分，`turn_speed` 就和上一題一樣。

![](https://i.imgur.com/epmMU1s.png)

接下來我就拿一個方向（`rho` < 0，代表線是往左斜的）來舉例，我在兩個方向裡面都定義了他們的 `dis_time`、`len_time`、`turn_time`，一開始有提到 `rho` 是原點到那條線的「長度」，而我們要先做的是讓車子趨近那條線，所以 `dis_time` 定義了車子要走幾秒才會到那條線。

靠近了那條線後，就是要轉彎讓車子做調整了，所以我利用上一題的概念定義了 `turn_time`，也定義了 `degree_by_x_axis`，因為在 `rho` 小於 0 時，90 < `theta` < 180，但其實車子只要把頭往回擺 180 - `theta` 度就好，所以在此 `degree_by_x_axis = 180 - theta`。

由於車輪畫一圈大約 65.493 cm，所以當我要轉 `degree_by_x_axis` 度時，就要轉 `65.493 x degree_by_x_axis / 360` 公分，再除上 `turn_speed` 代表車子要轉幾秒，而前面的 4 是一個經測試之後訂出來的調整的參數。

經過前面兩個 while 迴圈之後，車子就與線平行了，接下來我們就看看線是多長來決定要走多久。`len_time` 就是線的 `length` 除以 `turn_speed`，就是他要在線上走的時間，跑完這個 while 迴圈之後，車子就漂亮的沿著線校正完，並走到線的盡頭了。

![](https://i.imgur.com/nSdv64p.png)

## 2. Location Identification

首先是 AprilTag detection 的部分，我發現當 AprilTag 在鏡頭相對左邊時，他的 y_rotation 值換算成角度後會大概在 2 ~ 15 度之間，而當 AprilTag 在相對鏡頭右邊時，他的 y_rotation 值換算成角度後會大概在 345~359 度之間，所以我就用這些區間來判斷車子應該往哪個方向校正，如果介於 {2, 1, 0, 359, 358} 度的話我就判定為車子跟 AprilTag 是垂直的。

在判定好方位後，我就會用 uart 傳一個對應的字元給車子那邊吃，而且為了怕傳輸的頻率太高，導致車子像做小碎步一樣不停的校正，所以我兩秒才傳送一次，讓車子有時間可以往前進一點。

![](https://i.imgur.com/Lb5vLr6.png)

### Car behavior definition

首先我先定義了 `turn_my_carlib` 函式，這也是用來微調車子角度的函式，跟之前的那些 `turn_my` 函式的差別是他內輪的速度較快，其實也是可以透過調整參數來完成，不用重新定義，但這樣在意義上比較分明。

![](https://i.imgur.com/gG8XOoC.png)


在吃進 uart 傳來的訊號之後，便可以判斷車子該往哪轉，或是已經跟 AprilTag 垂直了要直走。若是傳進來的是 `l` 或 `r`，代表車子的角度要微調，所以就呼叫 `turn_my_carlib` 函式進行調整，當收到 `c` 的訊號時，則代表車子已經跟 AprilTag 垂直了，我就讓他再用 5cm/s 的速度走 0.5 秒就停。

## 3. XBee

我主要利用 XBee 來呼叫車子開始作業時的 RPC function，若輸入 `left` ，則車子會沿著左邊的線走到底，再進入 Location Identification 的部分，若輸入 `right` ，則車子會先沿著右邊的線走到底。

而在透過 Apriltag 把車子帶回來並停下後，也會透過 XBee 回傳 "OK" 的訊息。

---

## (2) what are the results

在 demo 時已線上 demo 給助教看了～
