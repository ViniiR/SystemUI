#[derive(Debug, Clone)]
pub enum HandlerError<'a> {
    DBusError,
    #[allow(dead_code)]
    ObjectError(&'a str),
}

pub struct Program;
impl Program {
    // Vinii's Graphical System Controller (Daemon)
    pub const PATH: &str = "/com/vinii/vgsc";
    pub const NAME: &str = "com.vinii.vgsc";

    pub const BACKEND_PATH: &str = "/com/vinii/vgsc";
    pub const BACKEND_NAME: &str = "com.vinii.vgsc";
}

// TODO: idk, but make it prettier, on dev side
pub mod dbus {
    pub struct Controllers;
    impl Controllers {
        pub const BRIGHTNESS: &str = "/com/vinii/vgsc/Brightness";
        pub const AUDIO: &str = "/com/vinii/vgsc/Audio";
        pub const BATTERY: &str = "/com/vinii/vgsc/Battery";
        pub const POWER: &str = "/com/vinii/vgsc/Power";

        pub fn to_interface(interface: &str) -> String {
            interface
                .chars()
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
