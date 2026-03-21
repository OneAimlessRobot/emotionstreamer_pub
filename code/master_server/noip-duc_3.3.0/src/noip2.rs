use std::fmt;
use std::fs;
use std::io;
use std::io::{Read, Seek};
use std::path::{Path, PathBuf};

use byteorder::{LittleEndian, ReadBytesExt};

type Result<T> = std::result::Result<T, Error>;

const OFFSET_MAGIC: u64 = 20;
const OFFSET_RLENGTH: u64 = 24;
const OFFSET_ELENGTH: u64 = 26;
const OFFSET_ENCRYPTED: usize = 30;
const OFFSET_REQ: usize = 48;

const MAGIC: u32 = 0x414a_324c;
const MAX_LEN: u16 = 4096;
const MIN_LEN: u16 = 20; // "username=&pass=".chars().count() + 6

/*
#define IPLEN  16
#define MAX_DEVLEN  16
struct CONFIG {
    char	lastIP[IPLEN];
    ushort	interval;	// don't move this (see display_current_config)
    ushort	chksum;
    uint	magic;
    ushort	rlength;
    ushort	elength;
    char	count;
    char	encrypt;
    char	nat;
    char	filler;
    char	device[MAX_DEVLEN];
    char	requests[0];
    char	execpath[0];
} *new_config = NULL;
*/
#[derive(Debug)]
pub struct Config {
    pub username: String,
    pub password: String,
    pub hostnames: Vec<String>,
    pub exec: Option<String>,
}

impl Config {
    fn parse(i: &[u8]) -> Result<Self> {
        let mut rdr = io::Cursor::new(i);

        // Check magic
        check_magic(&mut rdr)?;

        // Get requests length
        let rlen = get_rlength(&mut rdr)?;
        let elen = get_elength(&mut rdr)?;

        // Get whether the requests are "encrypted"
        let encrypt = get_encrypt(i)?;

        let (username, password, hostnames) = get_uph(&mut rdr, rlen, encrypt)?;

        Ok(Self {
            username,
            password,
            hostnames,
            exec: get_execpath(&mut rdr, rlen, elen, encrypt)?,
        })
    }
}

impl fmt::Display for Config {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "NOIP_USERNAME={}\nNOIP_PASSWORD={}\nNOIP_HOSTNAMES={}\nNOIP_EXEC_ON_CHANGE={}\n",
            self.username,
            self.password,
            self.hostnames.join(","),
            self.exec.as_ref().unwrap_or(&String::new()),
        )
    }
}

#[derive(Debug, thiserror::Error)]
pub struct ImportError {
    pub source: Error,
    pub filename: PathBuf,
}

impl fmt::Display for ImportError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "Failed to import noip2 config from '{}'; {}",
            self.filename.display(),
            self.source
        )
    }
}

#[derive(Debug, thiserror::Error)]
pub enum Error {
    #[error(transparent)]
    Base64(#[from] base64::DecodeError),

    #[error(transparent)]
    Utf8(#[from] std::str::Utf8Error),

    #[error(transparent)]
    Utf8String(#[from] std::string::FromUtf8Error),

    #[error("File does not appear to be a noip2 config due to incorrect magic bytes, 0x{0:08X}")]
    IncorrectMagic(u32),

    #[error(transparent)]
    ReadError(#[from] ReadError),

    #[error(transparent)]
    ValueError(#[from] ValueError),

    #[error("username is missing")]
    UsernameMissing,

    #[error("password ('pass' field) is missing")]
    PasswordMissing,
}

#[derive(Debug, thiserror::Error)]
pub enum ValueError {
    #[error("file data too short")]
    DataTooShort,

    #[error("encrypt flag has invalid value")]
    EncryptFlag,

    #[error("rlength appears invalid")]
    RLength(u16),

    #[error("rlength past end of file")]
    RLengthPastEof,

    #[error("elength appears invalid")]
    ELength(u16),

    #[error("elength past end of file")]
    ELengthPastEof,
}

#[derive(Debug, thiserror::Error)]
pub enum ReadError {
    #[error("could not open file; {0}")]
    Open(io::Error),

    #[error("could not read file; {0}")]
    ReadToEnd(io::Error),

    #[error("could not read magic bytes; {0}")]
    MagicBytes(io::Error),

    #[error("could not seek to end of file; {0}")]
    FindSize(io::Error),

    #[error("could not read elength bytes; {0}")]
    ELength(io::Error),

    #[error("could not read rlength bytes; {0}")]
    RLength(io::Error),
}

pub fn import(filename: &Path) -> std::result::Result<Config, ImportError> {
    let mut data = Vec::new();

    let _ = fs::File::open(filename)
        .map_err(|e| ImportError {
            source: ReadError::Open(e).into(),
            filename: filename.into(),
        })?
        .read_to_end(&mut data)
        .map_err(|e| ImportError {
            source: ReadError::ReadToEnd(e).into(),
            filename: filename.into(),
        })?;

    Config::parse(&data).map_err(|source| ImportError {
        source,
        filename: filename.into(),
    })
}

fn decode_to_bytes<T: std::convert::AsRef<[u8]>>(i: T, encrypt: bool) -> Result<Vec<u8>> {
    match encrypt {
        true => Ok(base64_decode(i)?),
        false => Ok(i.as_ref().into()),
    }
}

fn base64_decode<T: std::convert::AsRef<[u8]>>(bytes: T) -> Result<Vec<u8>> {
    use base64::Engine as _;

    base64::engine::general_purpose::STANDARD
        .decode(bytes)
        .map_err(Into::into)
}

fn decode_to_string<T: std::convert::AsRef<[u8]>>(i: T, encrypt: bool) -> Result<String> {
    let v = decode_to_bytes(i, encrypt)?;
    Ok(std::string::String::from_utf8(v)?)
}

const fn offset_execpath(rlength: u16) -> usize {
    OFFSET_REQ + rlength as usize
}

fn check_magic<T>(rdr: &mut io::Cursor<T>) -> Result<()>
where
    T: io::Read,
    io::Cursor<T>: io::Read,
{
    rdr.set_position(OFFSET_MAGIC);

    match rdr.read_u32::<LittleEndian>() {
        Ok(v) if v == MAGIC => Ok(()),
        Ok(v) => Err(Error::IncorrectMagic(v)),
        Err(e) => Err(ReadError::MagicBytes(e).into()),
    }
}

fn get_encrypt(i: &[u8]) -> Result<bool> {
    if OFFSET_ENCRYPTED >= i.len() {
        return Err(ValueError::DataTooShort.into());
    }

    Ok(match i[OFFSET_ENCRYPTED] {
        0 => false,
        1 => true,
        _ => return Err(ValueError::EncryptFlag.into()),
    })
}

fn get_data_len<T>(rdr: &mut io::Cursor<T>) -> Result<u64>
where
    T: io::Read,
    io::Cursor<T>: io::Read + io::Seek,
{
    rdr.seek(io::SeekFrom::End(0))
        .map_err(ReadError::FindSize)?;
    Ok(rdr.position())
}

fn get_rlength<T>(rdr: &mut io::Cursor<T>) -> Result<u16>
where
    T: io::Read,
    io::Cursor<T>: io::Read,
{
    rdr.set_position(OFFSET_RLENGTH);

    let rlen = match rdr.read_u16::<LittleEndian>() {
        Ok(v) if (MIN_LEN..=MAX_LEN).contains(&v) => Ok::<_, Error>(v),
        Ok(v) => Err(ValueError::RLength(v).into()),
        Err(e) => Err(ReadError::RLength(e).into()),
    }?;

    Ok(rlen)
}

fn get_elength<T>(rdr: &mut io::Cursor<T>) -> Result<u16>
where
    T: io::Read,
    io::Cursor<T>: io::Read + io::Seek,
{
    rdr.set_position(OFFSET_ELENGTH);

    match rdr.read_u16::<LittleEndian>() {
        Ok(v) if v <= MAX_LEN => Ok(v),
        Ok(v) => Err(ValueError::ELength(v).into()),
        Err(e) => Err(ReadError::ELength(e).into()),
    }
}

fn get_uph(
    rdr: &mut io::Cursor<&[u8]>,
    rlen: u16,
    encrypt: bool,
) -> Result<(String, String, Vec<String>)> {
    let mut username = None;
    let mut password = None;
    let mut hostnames = Vec::new();

    let start = OFFSET_REQ;
    let end = OFFSET_REQ + rlen as usize;

    if end > get_data_len(rdr)? as usize {
        return Err(ValueError::RLengthPastEof.into());
    }

    let req = decode_to_bytes(&rdr.get_ref()[start..end], encrypt)?;
    for (k, v) in form_urlencoded::parse(req.as_ref()) {
        //dbg!(&k, &v);
        match k.as_ref() {
            "username" => username = Some(v.to_string()),
            "pass" => password = Some(v.to_string()),
            "h[]" | "g[]" => hostnames.push(v.to_string()),
            _ => (),
        }
    }

    Ok((
        username.ok_or(Error::UsernameMissing)?,
        password.ok_or(Error::PasswordMissing)?,
        hostnames,
    ))
}

fn get_execpath(
    rdr: &mut io::Cursor<&[u8]>,
    rlen: u16,
    elen: u16,
    encrypt: bool,
) -> Result<Option<String>> {
    if elen == 0 {
        return Ok(None);
    }

    let start = offset_execpath(rlen);
    // The stored elength seems to include the null byte so subtract 1
    let end = start + elen as usize - 1;

    if end > get_data_len(rdr)? as usize {
        return Err(ValueError::ELengthPastEof.into());
    }

    Ok(Some(decode_to_string(&rdr.get_ref()[start..end], encrypt)?))
}

#[cfg(test)]
mod test {
    use std::io;
    use std::io::Read;

    // TODO: make a few from different client versions
    const FIXTURE: &'static str = "test-data/everything-2.1.7.conf";

    fn get_data() -> Vec<u8> {
        let mut data = Vec::new();
        let mut file = std::fs::File::open(FIXTURE).unwrap();
        file.read_to_end(&mut data).unwrap();
        data
    }

    #[test]
    fn can_decode_to_bytes_encrypt() {
        let data = "dGVzdA==";
        let res = super::decode_to_bytes(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!([116, 101, 115, 116], res.unwrap().as_slice());
    }

    #[test]
    fn can_decode_to_bytes_noencrypt() {
        let data = "test";
        let res = super::decode_to_bytes(&data.as_bytes(), false);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!([116, 101, 115, 116], res.expect("res").as_slice());
    }

    #[test]
    fn can_decode_to_bytes_encrypt_fails_on_invalid_base64() {
        let data = "dGVzd==";
        let res = super::decode_to_bytes(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_decode_to_bytes_encrypt_empty_vec_on_empty() {
        let data = "";
        let res = super::decode_to_bytes(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!(Vec::<u8>::new(), res.expect("res"));
    }

    #[test]
    fn can_decode_to_string_encrypt() {
        let data = "dGVzdA==";
        let res = super::decode_to_string(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!("test", res.unwrap());
    }

    #[test]
    fn can_decode_to_string_noencrypt() {
        let data = "test";
        let res = super::decode_to_string(&data.as_bytes(), false);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!("test", res.expect("res"));
    }

    #[test]
    fn can_decode_to_string_encrypt_fails_on_invalid_base64() {
        let data = "dGVzd==";
        let res = super::decode_to_string(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_decode_to_string_encrypt_empty_string_on_empty() {
        let data = "";
        let res = super::decode_to_string(&data.as_bytes(), true);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!(String::new(), res.expect("res"));
    }

    #[test]
    fn can_parse_noip2() {
        let c = super::import(std::path::Path::new(FIXTURE)).unwrap();
        assert!(c.exec.is_some())
    }

    #[test]
    fn can_check_magic() {
        let data = get_data();
        let mut rdr = io::Cursor::new(data.as_slice());
        let res = super::check_magic(&mut rdr);
        dbg!(&res);
        assert!(res.is_ok());
    }

    #[test]
    fn can_check_magic_is_incorrect() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::check_magic(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_encrypt() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
            9, 0,
        ];
        let res = super::get_encrypt(&data);
        dbg!(&res);
        assert!(res.is_ok());
    }

    #[test]
    fn can_get_encrypt_fails_on_too_short() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
            9,
        ];
        let res = super::get_encrypt(&data);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_data_len() {
        let data = [0, 1, 2, 3, 4];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_data_len(&mut rdr);
        dbg!(&res);
        assert!(res.is_ok());
        assert_eq!(5, res.expect("res"));
    }

    #[test]
    fn can_get_rlength() {
        let data = get_data();
        let mut rdr = io::Cursor::new(data.as_slice());
        assert_eq!(128, super::get_rlength(&mut rdr).unwrap());
    }

    #[test]
    fn can_get_rlength_data_too_short() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_rlength(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_rlength_value_too_low() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 0x01, 0x00,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_rlength(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_rlength_value_too_high() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 0x01, 0x10,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_rlength(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_elength() {
        let data = get_data();
        let mut rdr = io::Cursor::new(data.as_slice());
        assert_eq!(21, super::get_elength(&mut rdr).unwrap());
    }

    #[test]
    fn can_get_elength_data_too_short() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_elength(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    #[test]
    fn can_get_elength_value_too_high() {
        let data = [
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 0x01,
            0x10,
        ];
        let mut rdr = io::Cursor::new(&data[..]);
        let res = super::get_elength(&mut rdr);
        dbg!(&res);
        assert!(res.is_err());
    }

    // TODO: test get_uph
    // TODO: test get_execpath
}
