rm twconv_ref
#rm cluster3.tr8
#rm cluster3.trc
sudo g++ -o twconv_ref -Ofast -Wall twitter_ref.cc -std=c++20
sudo ./twconv_ref cluster26
#sudo chmod 777 cluster3.tr8
#sudo chmod 777 cluster3.trc