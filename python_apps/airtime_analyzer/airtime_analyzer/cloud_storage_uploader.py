import os
import logging
import uuid

from libcloud.storage.providers import get_driver
from libcloud.storage.types import Provider, ContainerDoesNotExistError, ObjectDoesNotExistError

class CloudStorageUploader:
    def __init__(self, provider, bucket, api_key, api_key_secret):
        self._provider = provider
        self._bucket = bucket
        self._api_key = api_key
        self._api_key_secret = api_key_secret

    def upload_obj(self, audio_file_path, metadata):
        file_base_name = os.path.basename(audio_file_path)
        file_name, extension = os.path.splitext(file_base_name)
        object_name = "%s_%s%s" % (file_name, str(uuid.uuid4()), extension)
        
        #cls = get_driver(getattr(Provider, self._provider))
        driver = self.get_cloud_driver()
        
        try:
            container = driver.get_container(self._bucket)
        except ContainerDoesNotExistError:
            container = driver.create_container(self._bucket)
        
        extra = {'meta_data': {'filename': file_base_name},
                 'acl': 'public-read-write'}
        
        with open(audio_file_path, 'rb') as iterator:
            obj = driver.upload_object_via_stream(iterator=iterator,
                                                  container=container,
                                                  object_name=object_name,
                                                  extra=extra)

        
        metadata["filesize"] = os.path.getsize(audio_file_path)
        
        '''remove file from organize directory'''
        try:
            os.remove(audio_file_path)
        except OSError:
            logging.info("Could not remove %s from organize directory" % audio_file_path)
        
        metadata["s3_object_name"] = object_name
        return metadata

    def delete_obj(self, obj_name):
        driver = self.get_cloud_driver()
        
        return_msg = dict()
        return_msg["success"] = False
        try:
            cloud_obj = driver.get_object(container_name=self._bucket,
                                    object_name=obj_name)
            return_msg["filesize"] = getattr(cloud_obj, 'size')
            return_msg["success"] = driver.delete_object(obj=cloud_obj)
            return return_msg
        except ObjectDoesNotExistError:
            logging.info("Could not find object on %s" % self._provider)

    def get_cloud_driver(self):
        cls = get_driver(getattr(Provider, self._provider))
        return cls(self._api_key, self._api_key_secret)
