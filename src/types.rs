#[derive(Debug, Clone)]
pub enum HandlerError {
    DBusError,
    ObjectError,
}

pub mod dbus {
    pub const DAEMON_NAME: Option<&str> = Some("VGSBackend");
    pub const INTERFACE: &str = "com.vinii.VGSController";

    pub struct Controllers;
    impl Controllers {
        pub const BRIGHTNESS: &str = "/com/vinii/BrightnessController";
        pub const AUDIO: &str = "/com/vinii/AudioController";
        pub const BATTERY: &str = "/com/vinii/BatteryController";
        pub const POWER: &str = "/com/vinii/PowerController";
    }

    pub struct Methods;
    impl Methods {
        pub const SET_BRIGHTNESS: &str = "SetBrightness";
        pub const GET_BRIGHTNESS: &str = "GetBrightness";
    }

    pub struct Timeout;
    impl Timeout {
        pub const NONE: i32 = -1;
    }
}
