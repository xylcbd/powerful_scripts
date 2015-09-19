#!/usr/bin/python
# -*- coding: UTF-8 -*-
import sys
import os
import thread
import urllib2

#global vars
process_count=0
total_count=0

#download files thread
def download_files(locker,thread_id,url_list,cache_dir,timeout):
	#used global vars
	global process_count
	global total_count
	locker.acquire()
	print("thread[%d] begin , len of url_list is %d , cache_dir is %s , timeout is %d ." % (thread_id,len(url_list),cache_dir,timeout))
	locker.release()
	for url in url_list:
		#increase count
		locker.acquire()
		process_count+=1
		print("progress %d/%d ..." % (process_count,total_count))
		locker.release()
		#do action
		tokens=url.split("/")
		filename=tokens[len(tokens)-1]
		filepath=os.path.join(cache_dir,filename)
		if os.path.exists(filepath):
			continue
		try:
			response=urllib2.urlopen(url,timeout=timeout)
			file = open(filepath,"wb")
			file.write(response.read())
			file.close()	
			response.close()
		except:
			pass
	#function done!
	locker.acquire()
	print("thread[%d] end." % (thread_id))
	locker.release()

def main_process(argv):
	#used global vars
	global process_count
	global total_count
	if len(argv) != 4:
		print("usage:\n\t"+argv[0]+" [thread_num] [input_url_list_file] [output_images_dir_path]")
		sys.exit()
	#get parameters
	thread_num = 0
	try:
		thread_num = int(argv[1],10)
	except:
		pass
	input_url_list_file = argv[2]
	output_images_dir_path = argv[3]
	locker = thread.allocate_lock()
	timeout = 10
	#check parameters
	if thread_num<=0:
		print("thread_num must be larger than zero!")
		sys.exit()
	if not os.path.exists(input_url_list_file) or not os.path.isfile(input_url_list_file):
		print("input_url_list_file is invalidate!")
		sys.exit()
	if not os.path.exists(output_images_dir_path) or not os.path.isdir(output_images_dir_path):
		print("output_images_dir_path is invalidate!")
		sys.exit()
	#do action
	urls = []
	file=open(input_url_list_file)
	for line in file:
		urls.append(line.strip())
	file.close()
	total_count=len(urls)
	print("total lines:"+str(total_count))
	if total_count<thread_num:
		thread_num=1
	count_per_thread=total_count/thread_num
	try:
		for i in range(0,thread_num):
			start_pos=i*count_per_thread
			stop_pos=start_pos+count_per_thread
			if stop_pos>total_count:
				stop_pos=total_count
			urls_per_thread=urls[start_pos : stop_pos]
			thread.start_new_thread(download_files,(locker,i,urls_per_thread,output_images_dir_path,timeout))
	except Exception as e:
		print e
		sys.exit()
	while True:
		if process_count>=total_count:
			break
		pass
	print("all done")
	
if __name__ == "__main__":
	try:
		main_process(sys.argv)
	except Exception as e:
		print(e)
	except KeyboardInterrupt:
		pass
	print("\nexit script!")

