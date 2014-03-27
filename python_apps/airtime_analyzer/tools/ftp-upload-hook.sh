#! /bin/bash

file_path=$1
filename="${file_path##*/}"

#base_instance_path and airtime_conf_path are common to all saas instances
base_instance_path=/mnt/airtimepro/instances/
airtime_conf_path=/etc/airtime/airtime.conf

#maps the instance_path to the url
vhost_file=/mnt/airtimepro/system/vhost.map

#instance_path will look like 1/1384, for example
instance_path=$(echo $file_path | grep -Po "(?<=($base_instance_path)).*?(?=/srv)")

#url to the airtime interface. bananas.airtime.pro, for example
url=http://
url+=$(grep -E $instance_path ~/vhost.map | awk '{print $1;}')
url+=/rest/media
#url=$(grep -E $instance_path $vhost_file | awk '{print $1;}')

#path to specific instance's airtime.conf
instance_conf_path=$base_instance_path$instance_path$airtime_conf_path

api_key=$(sudo awk -F "=" '/api_key/ {print $2}' /etc/airtime/airtime.conf)
#api_key=$(sudo awk -F "=" '/api_key/ {print $2}' $instance_conf_path)

#curl $url -u $api_key":" -X POST -F "file=@$file_path" -F "name=$filename"
