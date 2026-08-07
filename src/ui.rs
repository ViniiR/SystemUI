use gtk::{prelude::WidgetExt, Button};

pub fn update_button_active(btn: &Button, to_active: bool) {
    const ACTIVE: &str = "button-active";
    const INACTIVE: &str = "button-inactive";

    btn.add_css_class(if to_active { ACTIVE } else { INACTIVE });
    btn.remove_css_class(if to_active { INACTIVE } else { ACTIVE });
}

pub fn get_volume_icon(percentage: u32, is_muted: bool) -> String {
    if is_muted {
        return String::from("audio-volume-muted-symbolic");
    }

    match percentage {
        0..=40 => String::from("audio-status-volume-low-symbolic"),
        41..=60 => String::from("audio-status-volume-medium-symbolic"),
        _ => String::from("audio-status-volume-high-symbolic"),
    }
}
