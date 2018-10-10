source ./VERSION
cp README release/
chmod +x cfglibmysqlclient.sh
cp cfglibmysqlclient.sh release/
mv release tsced
chmod 755 tsced/module/*.so
chmod 755 tsced/module/script/*
tar zcvf ${V}.tar.gz tsced
mv tsced release
