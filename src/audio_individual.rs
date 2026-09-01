use gtk::Builder;
use gtk::{ffi::GtkWidget, gio::DBusConnection};

use crate::types::{AudioStream, HandlerError};

pub fn handle_audio_individual(
    builder: &Builder,
    conn: DBusConnection,
) -> Result<(), HandlerError<'_>> {
    Ok(())
}

fn list_item(stream: AudioStream) -> GtkWidget {
    todo!()
}
