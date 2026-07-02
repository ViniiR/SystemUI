use gtk::prelude::*;
use gtk::{glib, Application, ApplicationWindow};

fn main() -> glib::ExitCode {
    let app = Application::builder()
        .application_id("com.vinii.vgs")
        .build();

    app.connect_activate(activate);

    app.run()
}

fn activate(app: &Application) {
    let builder = gtk::Builder::from_file("src/ui/builder.ui");
    let window: ApplicationWindow = builder
        .object("main-window")
        .expect("Failed to create window in builder.ui");

    window.set_application(Some(app));

    window.present();
}
