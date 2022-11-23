#include <rpp/rpp.hpp>
#include <rppqt/rppqt.hpp>

#include <iostream>

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QVBoxLayout>

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

    rpp::source::interval(std::chrono::seconds{1})
            .tap([](const auto&)
            {
                std::cout << "Interval from thread: " << std::this_thread::get_id() << std::endl;
            })
            .combine_latest([](size_t index, size_t amount_of_clicks)
                            {
                                return QString("Seconds since start: %1 Clicks since start: %2").arg(index).arg(amount_of_clicks);
                            },
                            rppqt::source::from_signal(button, &QPushButton::pressed).scan(size_t{},
                                                                                           [](size_t seed, const auto&)
                                                                                           {
                                                                                               return seed + 1;
                                                                                           })
                                                                                     .tap([](const auto&)
                                                                                     {
                                                                                         std::cout << "Click from thread: " <<
                                                                                                 std::this_thread::get_id() << std::endl;
                                                                                     })
                                                                                     .start_with(size_t{0}))
            .subscribe_on(rpp::schedulers::new_thread{})
            .observe_on(rppqt::schedulers::main_thread_scheduler{})
            .subscribe([&](const QString& text)
            {
                std::cout << "Text updated from thread: " << std::this_thread::get_id() << std::endl;
                label.setText(text);
            });

    std::cout << "Application thread: " << std::this_thread::get_id() << std::endl;

    // There we have application's thread, "new_thread" where observable subscribed and interval events happened.
    // But we need to update GUI's objects in main thread, so, we forces observable to emit items to subscribe in QT's main thread
    // Example of output:
    /*
        Application thread: 19748       <----- main thread
        Interval from thread: 30604     <----- interval happens from another thread
        Text updated from thread: 19748 <----- but update then transfered to main thread
        Click from thread: 19748        <----- click happens from main thread due to QT logic
        Text updated from thread: 19748 <----- and update happens in main thread
        Interval from thread: 30604
        Text updated from thread: 19748
     */
    return app.exec();
}
