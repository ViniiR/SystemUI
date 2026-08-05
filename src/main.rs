use gtk::gdk::{self, Display};
use gtk::glib::g_critical;
use gtk::{gio, glib, Application, ApplicationWindow, EventControllerFocus, EventControllerKey};
use gtk::{prelude::*, CssProvider};

use crate::types::Program;

mod audio;
mod battery;
mod boost;
mod brightness;
mod conservation;
mod power;
mod types;
mod ui;

fn main() -> glib::ExitCode {
    let app = Application::builder().application_id(Program::NAME).build();

    app.connect_startup(|_| {
        let provider = CssProvider::new();
        provider.load_from_path("src/ui/style.css");

        let Some(display) = Display::default() else {
            g_critical!(None, "Failed to get default display");
            return;
        };

        gtk::style_context_add_provider_for_display(
            &display,
            &provider,
            gtk::STYLE_PROVIDER_PRIORITY_APPLICATION,
        );
    });
    app.connect_activate(activate);

    app.run()
}

fn activate(app: &Application) {
    let builder = gtk::Builder::from_file("src/ui/builder.ui");
    let Some(window) = builder.object::<ApplicationWindow>("main-window") else {
        g_critical!(None, "Failed to get builder main-window");
        return;
    };

    glib::spawn_future_local(async move {
        // NOTE: getting system bus
        let Ok(dbus_connection) = gio::bus_get_future(gio::BusType::System).await else {
            g_critical!(None, "Failed to connect with DBus");
            return;
        };

        if let Err(e) = brightness::handle_brightness(&builder, dbus_connection.clone()) {
            g_critical!(None, "Brightness error: {e:?}");
            return;
        };

        if let Err(e) = power::handle_power(&builder, dbus_connection.clone()) {
            g_critical!(None, "Power error: {e:?}");
            return;
        };

        if let Err(e) = battery::handle_battery(&builder, dbus_connection.clone()) {
            g_critical!(None, "Battery error: {e:?}");
            return;
        };

        if let Err(e) = boost::handle_boost(&builder, dbus_connection.clone()) {
            g_critical!(None, "Boost error: {e:?}");
            return;
        };

        if let Err(e) = conservation::handle_conservation(&builder, dbus_connection.clone()) {
            g_critical!(None, "Conservation error: {e:?}");
            return;
        };

        if let Err(e) = audio::handle_audio(&builder, dbus_connection.clone()) {
            g_critical!(None, "Audio error: {e:?}");
            return;
        };
    });

    let key_controller = EventControllerKey::new();
    key_controller.connect_key_pressed(glib::clone!(
        #[strong]
        window,
        move |_, val, _code, _state| {
            if val == gdk::Key::Escape {
                // TODO: fix critical error on esc press
                window.close();
                glib::Propagation::Stop
            } else {
                glib::Propagation::Proceed
            }
        }
    ));
    window.add_controller(key_controller);

    let focus_controller = EventControllerFocus::new();
    focus_controller.connect_leave(glib::clone!(
        #[strong]
        window,
        move |_| {
            window.close();
        }
    ));
    window.add_controller(focus_controller);

    window.set_application(Some(app));

    window.present();
}
