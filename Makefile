all:	qt/DiskUsageMonitor

qt/DiskUsageMonitor:
	cd qt && qmake DiskUsageMonitor.prj && make

diskUsageMonitor.deb:	qt/DiskUsageMonitor
	mkdir -p diskUsageMonitor/usr/bin diskUsageMonitor/opt/DiskUsageMonitor
	cp $^ diskUsageMonitor/opt/DiskUsageMonitor/
	sed -i 's/Version: .*/Version: 1.0.$(time)/' diskUsageMonitor/DEBIAN/control
	fakeroot dpkg --build diskUsageMonitor

