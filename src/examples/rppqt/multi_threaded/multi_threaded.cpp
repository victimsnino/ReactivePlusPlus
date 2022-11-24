#include <rpp/rpp.hpp>
#include <rppqt/rppqt.hpp>

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QVBoxLayout  vbox{};

    QLabel      label{};
    QPushButton button{"CLick me"};
    vbox.addWidget(&label);
    vbox.addWidget(&button);
    QMainWindow window{};

    window.setCentralWidget(new QWidget);
    window.centralWidget()->setLayout(&vbox);

    window.show();

    auto amount_of_clicks = rppqt::source::from_signal(button, &QPushButton::pressed)
                                .scan(size_t{}, [](size_t seed, const auto&) { return seed + 1; })
                                .start_with(size_t{})
                                .publish()
                                .ref_count();

    amount_of_clicks
        .observe_on(rpp::schedulers::new_thread{})
        .tap([](const auto&)
            {
                std::cout << "Some long computation...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds{1});
            })
        .observe_on(rppqt::schedulers::main_thread_scheduler{})
        .combine_latest([](size_t slow_clicks, size_t fast_clicks)
                        {
                            return QString{"Slow clicks are %1. Fast clicks are %2"}.arg(slow_clicks).arg(fast_clicks);
                        },
                        amount_of_clicks)
        .subscribe([&](const QString& text) { label.setText(text); });

    return app.exec();
}
