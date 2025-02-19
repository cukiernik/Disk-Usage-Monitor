#ifndef DISKUSAGEMONITOR_H
#define DISKUSAGEMONITOR_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QProcess>

class DiskUsageMonitor : public QWidget
{
    Q_OBJECT
public:
    explicit DiskUsageMonitor(QWidget *parent = nullptr);
    ~DiskUsageMonitor();

private slots:
    void refreshProcessList();

private:
    struct task{int pid; QString name;unsigned long long reads,writes;unsigned long long r,w;};
    QList<task> tasks;
    QVBoxLayout *layout;
    QLabel *processListLabel;
    QTimer *monitorTimer;
protected:
    void paintEvent(QPaintEvent *event) override ;
};

#endif // DISKUSAGEMONITOR_H
