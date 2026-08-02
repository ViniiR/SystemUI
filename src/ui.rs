use gtk::{prelude::WidgetExt, Button};

pub fn update_button_active(btn: &Button, to_active: bool) {
    const ACTIVE: &str = "button-active";
    const INACTIVE: &str = "button-inactive";

    btn.add_css_class(if to_active { ACTIVE } else { INACTIVE });
    btn.remove_css_class(if to_active { INACTIVE } else { ACTIVE });
}
