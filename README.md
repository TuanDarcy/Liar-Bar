1 bàn 4 người gồm 20 lá 
6 Q, 6 K , 6 ÁT , 2 JOKER 

ĐẠN NẠP 1 VIÊN TRONG 3 LỖ ĐẠN , TỈ LỆ CHẾT LÀ 1/3

Mỗi người phát 5 lá , hết bài 

tỉ lệ hệ thống chọn bài là 1/3 ( do k có joker )

trong từng lượt của mỗi người đánh tối đa 3 lá 

1. hướng dẫn chơi 
2. cách chơi 
3. cách chiến thắng 
4. chiến thắng tối ưu 


tỉ lệ sở hữu bài càng thấp thì tỉ lệ bị nghi ngờ càng cao ( 100%- tỉ lệ sở hữu)
p/s: tâm lý người chơi ngược lại với tỉ lệ sở hữu

chia 8 trường hợp ( đánh 3 lá trong 1 turn ) 
- 3 lá K thật ==>Thật 
- 2 lá K và 1 lá khác ==> Dối 
- 2 là K và 1 lá joker ==> Thật
- 1 lá K và 2 lá Joker ==> Thật
-1 lá K là 2 lá khác ==> Dối 
- 3 lá khác ===> Dối
- 1 lá K , 1 lá joker , 1 lá khác ==> dối 
- 2 lá joker , 1 lá khác ==> dối 
- 1 joker , 2 lá khác ==> dối 

chia 6 trường hợp ( đánh 2 lá 1 turn )
- 2 lá K ==> Thật 
- 2 lá Joker ==> Thật 
- 1 lá K , 1 lá joker ==> Thật 
-1 lá k , 1 lá  khác  ==> giả
-2 lá khác ==> giả
-1 lá joker , 1 lá khác  ==> Gỉa

chia 3 trường hợp  ( đánh 1 lá 1 turn )
-1 K ==>thật  
-1 JOker  ==>thật
-1 lá khác ==> giả 
Công thức Thật: (Số lá K + Số lá Joker) = Số lá đánh ra.
Công thức Dối: (Số lá K + Số lá Joker) < Số lá đánh ra.

Bạn có thể dùng công thức "Số K + Số Joker" để quét nhanh:
Thật: Khi bạn đánh ra n lá, và trong đó có k lá K, j lá Joker. Nếu $k + j = n$ thì là Thật.
Dối: Nếu k + j < n thì chắc chắn là Dối.

sẽ có 1  vòng  for trong main để kiểm tra số lá bài  của mình
sẽ có 1  lệnh if để kiểm tra tính đúng sai của turn bài mình đánh
nếu như  thỏa mãn i = 0 và  bool =  1 thì  người kế tiếp sẽ bị loại
nếu như  bool  =  0  thì mình sẽ bị  loại


class
-Player
-Alive
-LaBai
-

