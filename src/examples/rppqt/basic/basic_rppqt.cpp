#include <rpp/rpp.hpp>

#include <rppqt/rppqt.hpp>

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};

    auto button_1              = new QPushButton("Click me!");
    auto button_2              = new QPushButton("Click me!");
    auto clicks_count_label    = new QLabel();
    auto clicks_duration_label = new QLabel();

    const auto clicks_1      = rppqt::source::from_signal(*button_1, &QPushButton::clicked);
    const auto clicks_2      = rppqt::source::from_signal(*button_2, &QPushButton::clicked);
    const auto merged_clicks = clicks_1 | rpp::operators::merge_with(clicks_2);

    const auto total_clicks     = merged_clicks | rpp::operators::scan(0, [](int seed, auto) { return ++seed; });
    const auto click_times      = merged_clicks | rpp::operators::map([](auto) { return std::chrono::high_resolution_clock::now(); });
    const auto time_since_click = rpp::source::interval(std::chrono::milliseconds{1}, rppqt::schedulers::main_thread_scheduler{})
                                | rpp::operators::with_latest_from([](auto, const auto click_time) { return std::chrono::high_resolution_clock::now() - click_time; }, click_times);

    // .....

    total_clicks.subscribe([&clicks_count_label](int clicks) {
        clicks_count_label->setText(QString{"Clicked %1 times in total!"}.arg(clicks));
    });


    time_since_click.subscribe([&clicks_duration_label](std::chrono::high_resolution_clock::duration ms) {
        clicks_duration_label->setText(QString{"MS since last click %1!"}.arg(std::chrono::duration_cast<std::chrono::milliseconds>(ms).count()));
    });

    QMainWindow window{};
    auto        vbox = new QVBoxLayout();
    vbox->addWidget(button_1);
    vbox->addWidget(button_2);
    vbox->addWidget(clicks_count_label);
    vbox->addWidget(clicks_duration_label);
    window.setCentralWidget(new QWidget);
    window.centralWidget()->setLayout(vbox);
    window.show();

    return app.exec();
}
