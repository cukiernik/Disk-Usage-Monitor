all:	qt/DiskUsageMonitor

qt/DiskUsageMonitor:
	cd qt && qmake DiskUsageMonitor.prj && make

DiskUsageMonitor.deb:	qt/DiskUsageMonitor
	cp $^ diskUsageMonitor/opt/DiskUsageMonitor/
	#mv view-camera-4455/opt/view-camera-4455/serwer.a olej.napedowy/opt/olej.napedowy/olej.napedowy
	fakeroot dpkg --build diskUsageMonitor

