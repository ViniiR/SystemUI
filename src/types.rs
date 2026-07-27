#[derive(Debug, Clone)]
pub enum HandlerError {
    DBusError,
    ObjectError(&'static str),
}

// TODO: idk, but make it prettier, on the users side
pub mod dbus {
    pub const BUS_NAME: Option<&str> = Some("com.vinii.VGSController");

    pub struct Controllers;
    impl Controllers {
        pub const BRIGHTNESS: &str = "/com/vinii/VGSController/Brightness";
        pub const AUDIO: &str = "/com/vinii/VGSController/Audio";
        pub const BATTERY: &str = "/com/vinii/VGSController/Battery";
        pub const POWER: &str = "/com/vinii/VGSController/Power";

        pub fn to_interface(interface: &str) -> String {
            interface
                .chars()
                .into_iter()
                .skip(1)
                .collect::<String>()
                .replace('/', ".")
        }
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
