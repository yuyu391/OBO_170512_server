
kill -9 `ps aux|grep "https_web_server"| grep -v "grep"|awk '{print $2}'`
kill -9 `ps aux|grep "https_data_server"| grep -v "grep"|awk '{print $2}'`



cd  ./https_web_server/
./https_web_server &
cd ..
cd ./https_data_server/
./https_data_server &
cd ..
