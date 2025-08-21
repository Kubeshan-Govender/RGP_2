#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QRandomGenerator>
#include <QMessageBox>
#include <algorithm>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Car redbull{ui->redbullCar, ui->redbullCarPit, ui->redbullTimer};
    Car ferrari{ui->ferrariCar, ui->ferrariCarPit, ui->ferrariTimer};
    Car mercedes{ui->mercedesCar, ui->mercedesCarPit, ui->mercedesTimer};
    Car mclaren{ui->mclarenCar, ui->mclarenCarPit, ui->mclarenTimer};

    redbull.pitX = 0;
    ferrari.pitX = 290;
    mercedes.pitX = 590;
    mclaren.pitX = 870;

    auto generatePitLaps = []() {
        QList<int> pits;
        while (pits.size() < 5) {
            int lap = QRandomGenerator::global()->bounded(1, 52);
            if (!pits.contains(lap)) pits.append(lap);
        }
        return pits;
    };

    redbull.pitLaps = generatePitLaps();
    ferrari.pitLaps = generatePitLaps();
    mercedes.pitLaps = generatePitLaps();
    mclaren.pitLaps = generatePitLaps();

    cars = {redbull, ferrari, mercedes, mclaren};

    // hide pit cars &s initially
    for (Car &car : cars) {
        car.pitWidget->hide();
        car.timerLabel->hide();
    }

    setFixedSize(width(), height());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::moveCar(QWidget* car, int dx, int dy) {
    if (!car) return;

    int x = car->x();
    int y = car->y();

    car->move(x + dx, y + dy);
}

void MainWindow::onMoveTimerTimeout() {
    for (Car &car : cars) {
        switch (car.state) {
        case OnTrack: {
            int x = car.trackWidget->x();
            int y = car.trackWidget->y();

            if (x > -600) {
                moveCar(car.trackWidget, -70, 0);
            } else {
                // completed a lap
                car.lapsCompleted++;
                car.trackWidget->move(width() + 100, y);

                if (car.lapsCompleted >= 52) {
                    car.trackWidget->hide();
                    continue;
                }

                // pit check
                if (car.pitLaps.contains(car.lapsCompleted) && car.pitStopsDone < 5) {
                    car.pitStopsDone++;
                    car.state = EnteringPit;

                    // switch to pit car at right edge
                    car.trackWidget->hide();
                    car.pitWidget->show();
                    car.pitWidget->move(width() + 100, car.pitWidget->y());
                }
            }
            break;
        }

        case EnteringPit: {
            int px = car.pitWidget->x();
            if (px > car.pitX+50) {
                moveCar(car.pitWidget, -50, 0); // move left
            } else{
                // reached pit box
                car.state = InPit;

                // random pit duration
                car.pitDuration = QRandomGenerator::global()->bounded(1900, 2900);
                car.pitClock.restart();

                // show timer
                car.timerLabel->show();
            }
            break;
        }

        case InPit: {
            int elapsed = car.pitClock.elapsed();
            int remaining = car.pitDuration - elapsed;
            if (remaining > 0) {
                car.timerLabel->setText(QString::number(elapsed / 1000.0, 'f', 1) + "s");
            } else {
                // time done
                //car.timerLabel->hide();
                car.state = LeavingPit;
            }
            break;
        }

        case LeavingPit: {
            int px = car.pitWidget->x();
            if (px > -200) {
                moveCar(car.pitWidget, -30, 0); // keep driving left
            } else {
                // left pit lane, back to track
                car.pitWidget->hide();
                car.trackWidget->show();
                car.trackWidget->move(width() + 100, car.trackWidget->y());
                car.state = OnTrack;
            }
            break;
        }
        }
    }
    updateStandings();

    // check if all cars finished
    bool allFinished = std::all_of(cars.begin(), cars.end(),
                                   [](const Car &c){ return c.lapsCompleted >= 52; });

    if (allFinished) {
        QList<Car*> sorted;
        for (Car &c : cars) sorted.append(&c);

        std::sort(sorted.begin(), sorted.end(), [](Car* a, Car* b) {
            return a->lapsCompleted > b->lapsCompleted;
        });

        QString finalResults = "🏁 Final Race Results 🏁\n\n";
        int pos = 1;
        for (Car* c : sorted) {
            QString name;
            if (c->trackWidget == ui->redbullCar) name = "Red Bull";
            else if (c->trackWidget == ui->ferrariCar) name = "Ferrari";
            else if (c->trackWidget == ui->mercedesCar) name = "Mercedes";
            else if (c->trackWidget == ui->mclarenCar) name = "McLaren";

            finalResults += QString::number(pos++) + ". " + name + "\n";
        }

        ui->standingsLabel->setText(finalResults);
        moveTimer->stop(); // stop race loop
    }

}

void MainWindow::updateStandings() {
    // sort cars by laps first, then x position
    QList<Car*> sorted;
    for (Car &c : cars) sorted.append(&c);

    std::sort(sorted.begin(), sorted.end(), [](Car* a, Car* b) {
        if (a->lapsCompleted != b->lapsCompleted)
            return a->lapsCompleted > b->lapsCompleted; // more laps first
        return a->trackWidget->x() > b->trackWidget->x(); // further right is ahead
    });

    QString text = "Standings:\n";
    int pos = 1;
    for (Car* c : sorted) {
        QString name;
        if (c->trackWidget == ui->redbullCar) name = "Red Bull";
        else if (c->trackWidget == ui->ferrariCar) name = "Ferrari";
        else if (c->trackWidget == ui->mercedesCar) name = "Mercedes";
        else if (c->trackWidget == ui->mclarenCar) name = "McLaren";

        text += QString::number(pos++) + ". " + name +
                " (" + QString::number(c->lapsCompleted) + " laps)\n";
    }

    ui->standingsLabel->setText(text);
}


void MainWindow::on_start_clicked()
{
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, this, &MainWindow::onMoveTimerTimeout);
    moveTimer->start(30);

    ui->start->hide();
}

