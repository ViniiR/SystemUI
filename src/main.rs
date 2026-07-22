use gtk::{gdk::Display, glib::g_error};
use gtk::{gio, glib, Application, ApplicationWindow};
use gtk::{prelude::*, CssProvider};

mod brightness;
mod types;

pub const PROGRAM_NAME: &str = "com.vinii.vgs";

fn main() -> glib::ExitCode {
    let app = Application::builder().application_id(PROGRAM_NAME).build();

    app.connect_startup(|_| {
        let provider = CssProvider::new();
        provider.load_from_path("src/ui/style.css");

        gtk::style_context_add_provider_for_display(
            &Display::default().expect("Failed to connect to default display"),
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
        g_error!(None, "Failed to get builder main-window");
        return;
    };

    // TODO: improve error handling
    glib::MainContext::default().spawn_local(async move {
        let Ok(dbus_connection) = gio::bus_get_future(gio::BusType::Session).await else {
            g_error!(None, "Failed to connect with DBus");
            return;
        };

        if let Err(e) = brightness::handle_brightness(&builder, dbus_connection.clone()) {
            g_error!(None, "Brightness error: {:?}", e);
            return;
        };
    });

    window.set_application(Some(app));

    window.present();
}
