#include "DiskUsageMonitor.h"
#include <QProcess>
#include <QTimer>
#include <QStringList>
#include <QPainter>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QApplication>
#include <algorithm>
#include <fcntl.h>  // Dla O_RDONLY
#include <unistd.h> // Dla funkcji systemowych, jak open(), close()
#include <unistd.h>
#include <iostream>

DiskUsageMonitor::DiskUsageMonitor(QWidget *parent)
    : QWidget(parent)
{
    // Ustawienie okna
    setWindowFlags( Qt::WindowStaysOnTopHint );
    if(auto screen=QGuiApplication::primaryScreen()){
        auto size=screen->geometry();
        setGeometry(size.width()-size.width()/4, size.height()/2, size.width()/4, size.height()/2);
    }
    else resize(600, 400);
    layout = new QVBoxLayout(this);
    processListLabel = new QLabel(this);
    layout->addWidget(processListLabel);

    // Ustawienie timeru do co 5 sekund odświeżania listy procesów
    monitorTimer = new QTimer(this);
    monitorTimer->setSingleShot(true);
    connect(monitorTimer, &QTimer::timeout, this, &DiskUsageMonitor::refreshProcessList);

    // Inicjalizacja listy procesów
    refreshProcessList();
    setWindowTitle("Disk Usage Monitor");
    QDir::setCurrent(QString("/proc"));
    refreshProcessList();
}

DiskUsageMonitor::~DiskUsageMonitor()
{
    // Zwalnianie zasobów, jeśli są potrzebne
}

static void parse2(QString line,std::function<void(unsigned long long)> proc){
    QStringList l=line.split(":");
    if(l.count()>=2){
        bool ok;
        auto u=l[1].toULongLong(&ok);
        if(ok)proc(u);
    }
}

static unsigned long long maxOf(unsigned long long a,unsigned long long b){return (a>b)?a:b;}

void DiskUsageMonitor::refreshProcessList()
{
    QList<task> old; old.clear(); old=std::move(tasks);
    QDir proc("");
    for(auto&p:proc.entryList(QDir::Dirs| QDir::NoDotAndDotDot)){
        bool ok;
        task it={p.toInt(&ok),p,0,0,0,0};
        if(ok){
            int const io=open((p+"/"+"io").toUtf8().data(),O_RDONLY|O_CLOEXEC);
            if(-1!=io){
                char t[4096];int l=read(io,t,sizeof(t));
                ::close(io);
                if(l>0){
                    QStringList lines(QString::fromLocal8Bit(t,l-1).split("\n"));
                    parse2(lines[0],[&it](unsigned long long reads){it.reads+=reads;});
                    parse2(lines[1],[&it](unsigned long long writes){it.writes+=writes;});
                    parse2(lines[2],[&it](unsigned long long reads){it.reads+=reads;});
                    parse2(lines[3],[&it](unsigned long long writes){it.writes+=writes;});
                    parse2(lines[4],[&it](unsigned long long reads){it.reads+=reads;});
                    parse2(lines[5],[&it](unsigned long long writes){it.writes+=writes;});
                    for(auto o:old) if(it.pid==o.pid){
                        it.r=it.reads-o.reads;
                        it.w=it.writes-o.writes;
                        break;
                    }
                    int const cmdline=open((p+"/"+"cmdline").toUtf8().data(),O_RDONLY|O_CLOEXEC);
                    if(-1!=cmdline){
                        char t[256];int l=read(cmdline,t,sizeof(t));
                        ::close(cmdline);
                        if(l>0){
                            for(char*i=t+l-1;t<=i;i--)if(0==*i)*i=' ';
                            it.name=QString::fromLocal8Bit(t,l-1);
                        }
                    }
                    if((it.reads>0)||(it.writes>0))
                        tasks.append(it);
                }
            }
        }
    }
    std::sort(tasks.begin(), tasks.end(),[](const task& a, const task& b) {
        return maxOf(a.reads,a.writes) > maxOf(b.reads,b.writes);
    });
    update();
}

static void max(int&a,int b){if(a<b)a=b;}
static void max(unsigned long long&a,unsigned long long b){if(a<b)a=b;}

void DiskUsageMonitor::paintEvent(QPaintEvent *event) {
    int y=0; QPainter painter(this);
    int x[]={0,0,0};
    QFontMetrics fm(painter.font());
    unsigned long long reads_=1;   // unsigned long long writes_=1;
    for(auto&i:tasks){
        max(x[0],fm.horizontalAdvance(QString::number(i.reads)));
        max(x[1],fm.horizontalAdvance(QString::number(i.writes)));
        max(x[2],fm.horizontalAdvance(QString::number(i.pid)));
        max(reads_,i.r);
        max(reads_,i.w);
    }
    int const dy=fm.height(); x[0]+=dy/2; x[1]+=x[0]+dy/2;x[2]+=x[1]+dy/2;
    for(auto&i:tasks){
        painter.drawText(QRect(0, y,x[0], dy),Qt::AlignRight|Qt::AlignVCenter, QString::number(i.reads));
        painter.drawText(QRect(x[0], y,x[1]-x[0], dy),Qt::AlignRight|Qt::AlignVCenter, QString::number(i.writes));
        painter.drawText(QRect(x[1]+dy, y,contentsRect().right()-x[1], dy),Qt::AlignLeft|Qt::AlignVCenter, i.name);
        if(float m=((float)i.r)/reads_)
            painter.drawLine(x[0]-(x[0]-dy/2)*m,y+dy-2,x[0],y+dy-2);
        if(float m=((float)i.w)/reads_)
            painter.drawLine(x[1]-(x[1]-x[0]-dy/2)*m,y+dy-2,x[1],y+dy-2);
        y+=dy+1;
    }
    y=contentsRect().bottom() -dy;
    painter.drawText(QRect(0, y,x[0], dy),Qt::AlignRight|Qt::AlignBottom, QString("reads"));
    painter.drawText(QRect(x[0], y,x[1]-x[0], dy),Qt::AlignRight|Qt::AlignBottom, QString("writes"));
    monitorTimer->start(3000);
}
