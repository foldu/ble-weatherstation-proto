use structopt::StructOpt;
use uuid::Uuid;

/// Generate random UUID's in a format accepted by the NimBLE C library
#[derive(StructOpt)]
enum Opt {
    /// Convert an UUID into C source code
    UuidToC { uuid: Uuid },

    /// Generate a random UUID and print it as C source code
    RandomUuid,
}

/// Convert an UUID into C source code
fn uuid_to_c(uuid: &Uuid) -> String {
    let segments = uuid
        .as_bytes()
        .iter()
        .rev()
        .map(|b| format!("0x{:x}", b))
        .collect::<Vec<_>>()
        .join(",");

    format!("// UUID: {}\nBLE_UUID128_INIT({});", uuid, segments)
}

fn main() -> Result<(), anyhow::Error> {
    let opt = Opt::from_args();

    match opt {
        Opt::UuidToC { uuid } => {
            println!("{}", uuid_to_c(&uuid));
        }
        Opt::RandomUuid => {
            println!("{}", uuid_to_c(&Uuid::new_v4()));
        }
    }

    Ok(())
}
