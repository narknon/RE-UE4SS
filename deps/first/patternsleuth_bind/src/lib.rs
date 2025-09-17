#![allow(unused)]

use std::{error::Error, sync::Arc, time::Instant};
use patternsleuth_scanner::{Pattern, Xref};

use patternsleuth::resolvers::{Resolution, ResolverFactory, DynResolverFactory, resolvers};

use patternsleuth::resolvers::{
    futures::join,
    impl_collector,
    unreal::{
        engine_version::EngineVersion,
        fname::{FNameCtorWchar, FNameToString},
        ftext::FTextFString,
        gmalloc::GMalloc,
        guobject_array::GUObjectArray,
        static_construct_object::StaticConstructObjectInternal,
        fuobject_hash_tables::FUObjectHashTablesGet,
        kismet::GNatives,
        ConsoleManagerSingleton,
    },
    ResolveError,
};

impl_collector! {
    #[derive(Debug, PartialEq)]
    struct UE4SSResolution {
        guobject_array: GUObjectArray,
        fname_tostring: FNameToString,
        fname_ctor_wchar: FNameCtorWchar,
        gmalloc: GMalloc,
        static_construct_object_internal: StaticConstructObjectInternal,
        ftext_fstring: FTextFString,
        engine_version: EngineVersion,
        fuobject_hash_tables_get: FUObjectHashTablesGet,
        gnatives: GNatives,
        console_manager_singleton: ConsoleManagerSingleton,
    }
}

#[repr(C)]
pub struct LogLevel(extern "C" fn(*const u16));
impl LogLevel {
    fn log(&self, msg: impl AsRef<str>) {
        let wchar: Vec<u16> = msg.as_ref().encode_utf16().chain([0]).collect();
        (self.0)(wchar.as_ptr())
    }
}

#[repr(C)]
pub struct PsCtx {
    default: LogLevel,
    normal: LogLevel,
    verbose: LogLevel,
    warning: LogLevel,
    error: LogLevel,
    config: PsScanConfig,
}

macro_rules! _log_level {
    ($level:ident, $ctx:ident) => { $ctx.$level.log("") };
    ($level:ident, $ctx:ident, $($arg:tt)*) => { $ctx.$level.log(format!($($arg)*)) };
}
macro_rules! default { ($ctx:ident $($arg:tt)*) => { _log_level!(default, $ctx $($arg)*) }; }
macro_rules! normal { ($ctx:ident $($arg:tt)*) => { _log_level!(normal, $ctx $($arg)*) }; }
macro_rules! verbose { ($ctx:ident $($arg:tt)*) => { _log_level!(verbose, $ctx $($arg)*) }; }
macro_rules! warning { ($ctx:ident $($arg:tt)*) => { _log_level!(warning, $ctx $($arg)*) }; }
macro_rules! error { ($ctx:ident $($arg:tt)*) => { _log_level!(error, $ctx $($arg)*) }; }

#[repr(C)]
pub struct PsEngineVersion {
    major: u16,
    minor: u16,
}

#[repr(C)]
pub struct PsScanConfig {
    guobject_array: bool,
    fname_tostring: bool,
    fname_ctor_wchar: bool,
    gmalloc: bool,
    static_construct_object_internal: bool,
    ftext_fstring: bool,
    engine_version: bool,
    fuobject_hash_tables_get: bool,
    gnatives: bool,
    console_manager_singleton: bool,
}

#[repr(C)]
pub struct PsScanResults {
    guobject_array: u64,
    fname_tostring: u64,
    fname_ctor_wchar: u64,
    gmalloc: u64,
    static_construct_object_internal: u64,
    ftext_fstring: u64,
    engine_version: PsEngineVersion,
    fuobject_hash_tables_get: u64,
    gnatives: u64,
    console_manager_singleton: u64,
}

#[derive(Debug, Default)]
struct ScanErrors(Vec<Box<dyn Error>>);
impl std::error::Error for ScanErrors {}
impl std::fmt::Display for ScanErrors {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "ScanError")
    }
}

pub fn ps_scan_internal(ctx: &PsCtx, results: &mut PsScanResults) -> Result<(), Box<dyn Error>> {
    default!(ctx, "Reading image");

    let exe = patternsleuth::process::internal::read_image()?;

    default!(ctx, "Starting scan");
    let before = Instant::now();
    let resolution = exe.resolve(UE4SSResolution::resolver())?;
    default!(ctx, "Scan finished in {:?}", before.elapsed());

    let mut errors = ScanErrors::default();

    macro_rules! handle {
        ($member:ident, $name:literal, $lua:literal) => {
            handle!($member, $name, $lua, false);
        };
        ($member:ident, $name:literal, $lua:literal, $optional:expr) => {
            if ctx.config.$member {
                match resolution.$member {
                    Ok(res) => {
                        default!(ctx, "Found {}: 0x{:x?}", $name, res.0);
                        results.$member = res.0;
                    }
                    Err(err) => {
                        warning!(ctx, "Failed to find {}: {err}", $name);
                        warning!(
                            ctx,
                            "You can supply your own AOB in 'UE4SS_Signatures/{}'",
                            $lua
                        );
                        results.$member = 0;
                        // Only add to `errors` if it's not optional:
                        if !$optional {
                            errors.0.push(Box::new(err));
                        }
                    }
                }
            }
        };
    }
    if ctx.config.engine_version {
        match resolution.engine_version {
            Ok(res) => {
                default!(ctx, "Found EngineVersion: {}", res);
                results.engine_version.major = res.major;
                results.engine_version.minor = res.minor;
            }
            Err(err) => {
                warning!(ctx, "Failed to find EngineVersion: {err}");
                warning!(
                    ctx,
                    "You need to override the engine version in 'UE4SS-settings.ini'."
                );
                errors.0.push(Box::new(err));
            }
        }
    }
    handle!(guobject_array, "GUObjectArray", "GUObjectArray.lua");
    handle!(gmalloc, "GMalloc", "GMalloc.lua");
    handle!(fname_tostring, "FName::ToString", "FName_ToString.lua");
    handle!(
        fname_ctor_wchar,
        "FName::FName(wchar_t*)",
        "FName_Constructor.lua"
    );
    handle!(
        static_construct_object_internal,
        "StaticConstructObject_Internal",
        "StaticConstructObject.lua"
    );
    handle!(
        ftext_fstring,
        "FText::FText(FString&&)",
        "FText_Constructor.lua",
        true
    );
    
    handle!(
        fuobject_hash_tables_get,
        "FUObjectHashTables::Get()",
        "GUObjectHashTables.lua",
        true
    );
    
    handle!(
        gnatives,
        "GNatives",
        "GNatives.lua",
        true
    );
    
    handle!(
        console_manager_singleton,
        "ConsoleManagerSingleton",
        "ConsoleManager.lua",
        true
    );

    if errors.0.is_empty() {
        Ok(())
    } else {
        Err(Box::new(errors))
    }
}

#[no_mangle]
pub extern "C" fn ps_scan(ctx: &PsCtx, results: &mut PsScanResults) -> bool {
    if let Err(_err) = ps_scan_internal(ctx, results) {
        warning!(ctx, "Scan failed\n");
        false
    } else {
        true
    }
}
 

//  // Don't forget to get vtable size
//  uint64_t vtable_size = ps_get_vtable_size(0x7FF612345678);
#[no_mangle]
pub extern "C" fn ps_get_vtable_size(vtable_address: u64) -> u64 {
    match patternsleuth::process::internal::read_image() {
        Ok(exe) => {
            let mut walker = patternsleuth::vtable_walker::VTableWalker::new(&exe);
            match walker.walk_vtable(vtable_address) {
                Ok(vtable) => vtable.estimated_size as u64,
                Err(_) => 0,
            }
        }
        Err(_) => 0,
    }
}

// // Pattern scanning
//  uint64_t* pattern_results;
//  size_t pattern_count;
//  if (ps_scan_pattern("48 89 5C 24 ?? 48 89 74", &pattern_results, &pattern_count)) {
//      for (size_t i = 0; i < pattern_count; i++) {
//          printf("Found pattern at: 0x%llx\n", pattern_results[i]);
//      }
//      ps_free_results(pattern_results, pattern_count);
//  }

/// Scan for an arbitrary pattern
/// Returns array of addresses where pattern was found, caller must free with ps_free_results
#[no_mangle]
pub extern "C" fn ps_scan_pattern(pattern_str: *const i8, results: *mut *mut u64, count: *mut usize) -> bool {
    if pattern_str.is_null() || results.is_null() || count.is_null() {
        return false;
    }

    let pattern_str = unsafe { std::ffi::CStr::from_ptr(pattern_str) };
    let pattern_str = match pattern_str.to_str() {
        Ok(s) => s,
        Err(_) => return false,
    };

    let pattern = match Pattern::new(pattern_str) {
        Ok(p) => p,
        Err(_) => return false,
    };

    let exe = match patternsleuth::process::internal::read_image() {
        Ok(exe) => exe,
        Err(_) => return false,
    };

    let mut all_results = Vec::new();
    for section in exe.memory.sections() {
        let scan_results = patternsleuth_scanner::scan_pattern(
            &[&pattern],
            section.address() as usize,
            section.data()
        );
        all_results.extend(scan_results[0].iter().map(|&addr| addr as u64));
    }

    unsafe {
        *count = all_results.len();
        if all_results.is_empty() {
            *results = std::ptr::null_mut();
        } else {
            let buffer = all_results.as_mut_ptr();
            std::mem::forget(all_results);
            *results = buffer;
        }
    }

    true
}

//  // String scanning
//  uint64_t* string_results;
//  size_t string_count;
//  if (ps_scan_string("GUObjectArray", &string_results, &string_count)) {
//      for (size_t i = 0; i < string_count; i++) {
//          printf("Found string at: 0x%llx\n", string_results[i]);
//      }
//      ps_free_results(string_results, string_count);
//  }
//

/// Scan for a string (UTF-8)
/// Returns array of addresses where string was found
#[no_mangle]
pub extern "C" fn ps_scan_string(search_str: *const i8, results: *mut *mut u64, count: *mut usize) -> bool {
    if search_str.is_null() || results.is_null() || count.is_null() {
        return false;
    }

    let search_str = unsafe { std::ffi::CStr::from_ptr(search_str) };
    let search_bytes = search_str.to_bytes();

    let pattern = match Pattern::from_bytes(search_bytes.to_vec()) {
        Ok(p) => p,
        Err(_) => return false,
    };

    let exe = match patternsleuth::process::internal::read_image() {
        Ok(exe) => exe,
        Err(_) => return false,
    };

    let mut all_results = Vec::new();
    for section in exe.memory.sections() {
        let scan_results = patternsleuth_scanner::scan_pattern(
            &[&pattern],
            section.address() as usize,
            section.data()
        );
        all_results.extend(scan_results[0].iter().map(|&addr| addr as u64));
    }

    unsafe {
        *count = all_results.len();
        if all_results.is_empty() {
            *results = std::ptr::null_mut();
        } else {
            let buffer = all_results.as_mut_ptr();
            std::mem::forget(all_results);
            *results = buffer;
        }
    }

    true
}


//  // Wide string scanning
//  const wchar_t* search = L"UKismetStringTableLibrary";
//  uint64_t* wstring_results;
//  size_t wstring_count;
//  if (ps_scan_wstring((const uint16_t*)search, &wstring_results, &wstring_count)) {
//      for (size_t i = 0; i < wstring_count; i++) {
//          printf("Found wide string at: 0x%llx\n", wstring_results[i]);
//      }
//      ps_free_results(wstring_results, wstring_count);
//  }

/// Scan for a wide string (UTF-16)
/// Returns array of addresses where string was found
#[no_mangle]
pub extern "C" fn ps_scan_wstring(search_str: *const u16, results: *mut *mut u64, count: *mut usize) -> bool {
    if search_str.is_null() || results.is_null() || count.is_null() {
        return false;
    }

    // Find null terminator
    let mut len = 0;
    unsafe {
        while *search_str.add(len) != 0 {
            len += 1;
        }
    }

    let search_slice = unsafe { std::slice::from_raw_parts(search_str, len) };
    let search_bytes: Vec<u8> = search_slice.iter()
        .flat_map(|&c| c.to_le_bytes())
        .collect();

    let pattern = match Pattern::from_bytes(search_bytes) {
        Ok(p) => p,
        Err(_) => return false,
    };

    let exe = match patternsleuth::process::internal::read_image() {
        Ok(exe) => exe,
        Err(_) => return false,
    };

    let mut all_results = Vec::new();
    for section in exe.memory.sections() {
        let scan_results = patternsleuth_scanner::scan_pattern(
            &[&pattern],
            section.address() as usize,
            section.data()
        );
        all_results.extend(scan_results[0].iter().map(|&addr| addr as u64));
    }

    unsafe {
        *count = all_results.len();
        if all_results.is_empty() {
            *results = std::ptr::null_mut();
        } else {
            let buffer = all_results.as_mut_ptr();
            std::mem::forget(all_results);
            *results = buffer;
        }
    }

    true
}

//  // Xref scanning
//  uint64_t* xref_results;
//  size_t xref_count;
//  if (ps_scan_xref(0x7FF612345678, &xref_results, &xref_count)) {
//      for (size_t i = 0; i < xref_count; i++) {
//          printf("Found xref at: 0x%llx\n", xref_results[i]);
//      }
//      ps_free_results(xref_results, xref_count);
//  }

/// Scan for cross-references to a specific address
/// Returns array of addresses that reference the target
#[no_mangle]
pub extern "C" fn ps_scan_xref(target_address: u64, results: *mut *mut u64, count: *mut usize) -> bool {
    if results.is_null() || count.is_null() {
        return false;
    }

    let exe = match patternsleuth::process::internal::read_image() {
        Ok(exe) => exe,
        Err(_) => return false,
    };

    let xref = Xref(target_address as usize);
    let mut all_results = Vec::new();

    for section in exe.memory.sections() {
        let scan_results = patternsleuth_scanner::scan_xref(
            &[&xref],
            section.address() as usize,
            section.data()
        );
        all_results.extend(scan_results[0].iter().map(|&addr| addr as u64));
    }

    unsafe {
        *count = all_results.len();
        if all_results.is_empty() {
            *results = std::ptr::null_mut();
        } else {
            let buffer = all_results.as_mut_ptr();
            std::mem::forget(all_results);
            *results = buffer;
        }
    }

    true
}

/// Free results array returned by scan functions
#[no_mangle]
pub extern "C" fn ps_free_results(results: *mut u64, count: usize) {
    if !results.is_null() && count > 0 {
        unsafe {
            Vec::from_raw_parts(results, count, count);
        }
    }
}

// Single resolver call (still uses batch system internally for caching benefits)
//uint64_t guobject_array = ps_resolve_single("GUObjectArray");
//uint64_t gmalloc = ps_resolve_single("GMalloc");

/// Call a single resolver by name
/// Returns the resolved address, or 0 on failure
#[no_mangle]
pub extern "C" fn ps_resolve_single(resolver_name: *const i8) -> u64 {
    if resolver_name.is_null() {
        return 0;
    }

    let name = unsafe { std::ffi::CStr::from_ptr(resolver_name) };
    let name = match name.to_str() {
        Ok(s) => s,
        Err(_) => return 0,
    };

    // Find the resolver by name
    let resolver = match resolvers().find(|r| r.name == name) {
        Some(r) => r,
        None => return 0,
    };

    let exe = match patternsleuth::process::internal::read_image() {
        Ok(exe) => exe,
        Err(_) => return 0,
    };

    // Resolve using the batch system (even for single resolver)
    match exe.resolve_many(&[resolver.getter]) {
        results if !results.is_empty() => {
            match &results[0] {
                Ok(resolution) => {
                    // Extract address from the resolution
                    resolution.get().unwrap_or(0)
                }
                Err(_) => 0,
            }
        }
        _ => 0,
    }
}

// Batch resolve - null terminated array
//const char* resolver_names[] = {
//  "GUObjectArray",
//  "GMalloc",
//  "FNameToString",
//  "EngineVersion",
//  nullptr  // null terminator
//};
//
//uint64_t results[4] = {0};
//size_t count = ps_resolve_batch(resolver_names, results);
//
//for (size_t i = 0; i < count; i++) {
//  printf("%s: 0x%llx\n", resolver_names[i], results[i]);
//}

/// Batch resolve multiple resolvers
/// resolver_names: null-terminated array of resolver name strings
/// results: output array that will be filled with addresses (0 on failure)
/// Returns the number of resolvers processed
#[no_mangle]
pub extern "C" fn ps_resolve_batch(
  resolver_names: *const *const i8,
  results: *mut u64,
) -> usize {
  if resolver_names.is_null() || results.is_null() {
      return 0;
  }

  let exe = match patternsleuth::process::internal::read_image() {
      Ok(exe) => exe,
      Err(_) => return 0,
  };

  // Collect resolver names until we hit null
  let mut names = Vec::new();
  let mut i = 0;
  loop {
      let name_ptr = unsafe { *resolver_names.add(i) };
      if name_ptr.is_null() {
          break;
      }

      let name = unsafe { std::ffi::CStr::from_ptr(name_ptr) };
      if let Ok(s) = name.to_str() {
          names.push((i, s));
      }
      i += 1;
  }

  // Collect resolver factories
  let mut resolver_getters = Vec::new();
  let mut resolver_indices = Vec::new();

  for (idx, name) in &names {
      match resolvers().find(|r| r.name == *name) {
          Some(resolver) => {
              resolver_getters.push(resolver.getter);
              resolver_indices.push(*idx);
          }
          None => {
              unsafe { *results.add(*idx) = 0; }
          }
      }
  }

  // Batch resolve all at once
  let batch_results = exe.resolve_many(&resolver_getters);

  // Fill in results
  for (idx, resolution) in resolver_indices.iter().zip(batch_results.iter()) {
      let addr = match resolution {
          Ok(res) => res.get().unwrap_or(0),
          Err(_) => 0,
      };
      unsafe { *results.add(*idx) = addr; }
  }

  names.len()
}

//// Get available resolvers
//const char* available_names[100];
//size_t resolver_count;
//if (ps_get_resolver_names(available_names, &resolver_count)) {
//    for (size_t i = 0; i < resolver_count; i++) {
//        printf("Available resolver: %s\n", available_names[i]);
//    }
//}

/// Get list of available resolver names
/// names: output array of string pointers (do not free these)
/// count: output count of resolvers
#[no_mangle]
pub extern "C" fn ps_get_resolver_names(names: *mut *const i8, count: *mut usize) -> bool {
    if names.is_null() || count.is_null() {
        return false;
    }

    let resolver_list: Vec<_> = resolvers()
        .map(|r| r.name.as_ptr() as *const i8)
        .collect();

    unsafe {
        *count = resolver_list.len();
        std::ptr::copy_nonoverlapping(resolver_list.as_ptr(), names, resolver_list.len());
    }

    true
}

//// Check if resolver exists
//if (ps_resolver_exists("GUObjectArray")) {
//    // resolver is available
//}

/// Helper to check if a resolver exists
#[no_mangle]
pub extern "C" fn ps_resolver_exists(resolver_name: *const i8) -> bool {
    if resolver_name.is_null() {
        return false;
    }

    let name = unsafe { std::ffi::CStr::from_ptr(resolver_name) };
    match name.to_str() {
        Ok(s) => resolvers().any(|r| r.name == s),
        Err(_) => false,
    }
}
