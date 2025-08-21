#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum CarState { OnTrack, EnteringPit, InPit, LeavingPit };

struct Car {
    QWidget* trackWidget;    // car on track
    QWidget* pitWidget;      // car in pit lane
    QLabel* timerLabel;      // timer display
    int lapsCompleted = 0;
    int pitStopsDone = 0;
    QList<int> pitLaps;
    CarState state = OnTrack;
    int pitX;                // parking X position (0, 290, 590, 870)
    int pitDuration = 0;     // milliseconds for current stop
    QElapsedTimer pitClock;  // measures elapsed time in pit
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    QTimer *moveTimer;
    void moveCar(QWidget* car, int dx, int dy);
    QList<Car> cars;


private slots:
    void onMoveTimerTimeout();
    void updateStandings();
    void on_start_clicked();
};

#endif // MAINWINDOW_H
