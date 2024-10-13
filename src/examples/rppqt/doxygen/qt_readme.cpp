#include <rpp/rpp.hpp>

#include <rppqt/rppqt.hpp>

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>

/**
 * @example qt_readme.cpp
 */

int main(int argc, char** argv) // NOLINT(bugprone-exception-escape)
{
    QApplication app{argc, argv};

    // ![readme]
    auto button = new QPushButton("Click me!");
    auto label  = new QLabel();
    rppqt::source::from_signal(*button, &QPushButton::clicked) // <------ react on signals
        | rpp::operators::observe_on(rpp::schedulers::new_thread{})
        | rpp::ops::tap([](int) { std::this_thread::sleep_for(std::chrono::milliseconds{500}); }) // some heavy job
        | rpp::operators::scan(0, [](int seed, auto) { return ++seed; })
        | rpp::operators::observe_on(rppqt::schedulers::main_thread_scheduler{}) // <--- go back to main QT scheduler
        | rpp::operators::subscribe([&label](int clicks) {
              label->setText(QString{"Clicked %1 times in total!"}.arg(clicks));
        });
    // ![readme]

    QMainWindow window{};
    auto        vbox = new QVBoxLayout();
    vbox->addWidget(button);
    vbox->addWidget(label);
    window.setCentralWidget(new QWidget);
    window.centralWidget()->setLayout(vbox);
    window.show();


    return app.exec();
}
