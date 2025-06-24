var doctors = [];

var currentEditingDoctorOriginalName = null;
let confirmHandler = null;

const modalTitleEl = document.getElementById("modal-title");
const saveButtonEl = document.getElementById("save-doctor-btn");
const deleteButtonEl = document.getElementById("delete-doctor-btn");

function createDoctorCard(doctor){
    const defaultPhotoUrl = "user.png";
    const photoUrl = doctor.photo || defaultPhotoUrl;

    const wrapper_card = document.createElement("div");
    const data_section = document.createElement("div");
    const button_section = document.createElement("div");

    const img_part = document.createElement("div");
    const txt_part = document.createElement("div");

    const name_txt = document.createElement("p");
    const description_txt = document.createElement("p");

    wrapper_card.classList.add("flex", "items-center", "gap-4", "bg-white", "px-4", "min-h-[72px]", "py-2", "justify-between");
    data_section.classList.add("flex", "items-center", "gap-4");
    button_section.classList.add("shrink-0");

    img_part.classList.add("bg-center", "bg-no-repeat", "aspect-square", "bg-cover", "rounded-full", "h-14", "w-fit");
    img_part.setAttribute("style", `background-image: url('${photoUrl}')`);
    txt_part.classList.add("flex", "flex-col", "justify-center");

    name_txt.classList.add("text-[#111718]", "text-base", "font-medium", "leading-normal", "line-clamp-1");
    name_txt.textContent = doctor.name || "N/A Name";

    description_txt.classList.add("text-[#638488]", "text-sm", "font-normal", "leading-normal", "line-clamp-2");
    const doc_shift_pref = doctor.shift_preference;
    description_txt.textContent = (doctor.specialty.toUpperCase() + " - " + doctor.max_shift + " shift maks - " + (doc_shift_pref.charAt(0).toUpperCase() + doc_shift_pref.slice(1)) + " - Alokasi " + doctor.allocated_shift + " shift - Pelanggaran Preferensi " + doctor.preference_violation + " kali") || "N/A Specialty atau N/A Maks Shift atau N/A Shift Preference atau N/A Alokasi Shift";

    txt_part.appendChild(name_txt);
    txt_part.appendChild(description_txt);

    data_section.appendChild(img_part);
    data_section.appendChild(txt_part);

    const edit_button = document.createElement("button");
    edit_button.classList.add("text-base", "font-medium", "leading-normal", "text-[#111718]", "hover:text-[#515451]");
    edit_button.textContent = "Edit";

    edit_button.addEventListener('click', () => {
        openEditDoctorModal(doctor);
    });

    button_section.appendChild(edit_button);

    wrapper_card.appendChild(data_section);
    wrapper_card.appendChild(button_section);

    return wrapper_card;
}

const doctors_list_container = document.getElementById("doctors-list-container");

const add_doctor_btn = document.getElementById("add-doctor-btn");
const add_doctor_modal = document.getElementById("add-doctor-modal");
const close_modal_btn = document.getElementById("close-modal-btn");

const first_name_el = document.getElementById("first-name");
const last_name_el = document.getElementById("last-name");
const birthdate_el = document.getElementById("birthdate");
const phone_el = document.getElementById("phone-number");
const address_el = document.getElementById("address");
const specialty_el = document.getElementById("specialty");
const max_shift_el = document.getElementById("max-shift");
const custom_shift_input = document.getElementById("custom-shift-input");
const shift_preference_el = document.getElementById("shift-preference");

const warning_txt = document.getElementById("warning-text");

if (add_doctor_btn) add_doctor_btn.addEventListener("click", () => {
    currentEditingDoctorOriginalName = null;
    clearFields();
    
    // Set UI for Add mode
    if (modalTitleEl) modalTitleEl.textContent = "Tambah Dokter Baru";
    if (saveButtonEl) saveButtonEl.textContent = "Simpan";
    if (deleteButtonEl) deleteButtonEl.classList.add("hidden");
    
    if (warning_txt) warning_txt.classList.add("hidden");
    if (add_doctor_modal) add_doctor_modal.classList.remove("hidden");
});

const confirmation_modal = document.getElementById("confirmation-modal");
const confirmation_txt = document.getElementById("confirmation-txt");
const confirm_no_btn = document.getElementById("confirm-no-btn");
const confirm_yes_btn = document.getElementById("confirm-yes-btn");

function openEditDoctorModal(doctorData) {
    currentEditingDoctorOriginalName = doctorData.name;

    if (modalTitleEl) modalTitleEl.textContent = `Edit Dokter: ${doctorData.name}`;
    if (saveButtonEl) saveButtonEl.textContent = "Perbarui";
    if (deleteButtonEl) deleteButtonEl.classList.remove("hidden");

    const nameParts = (doctorData.name || "").split(" ");
    if (first_name_el) first_name_el.value = nameParts[0] || "";
    if (last_name_el) last_name_el.value = nameParts.slice(1).join(" ") || "";

    if (birthdate_el) birthdate_el.value = doctorData.birthdate || "";
    if (phone_el) phone_el.value = doctorData.phone || "";
    if (address_el) address_el.value = doctorData.address || "";
    if (specialty_el) specialty_el.value = doctorData.specialty || "Pilih spesialisasi";
    
    if (max_shift_el) {
        let maxShiftValue = String(doctorData.max_shift);
        const options = Array.from(max_shift_el.options).map(opt => opt.value);
        if (options.includes(maxShiftValue)) {
            max_shift_el.value = maxShiftValue;
            if (custom_shift_input) custom_shift_input.classList.add("hidden");
        } else {
            max_shift_el.value = "custom-shift";
            if (custom_shift_input) {
                custom_shift_input.classList.remove("hidden");
                custom_shift_input.value = maxShiftValue;
            }
        }
    }
    if (shift_preference_el) shift_preference_el.value = doctorData.shift_preference || "Pilih preferensi shift";

    if (warning_txt) warning_txt.classList.add("hidden");
    if (add_doctor_modal) add_doctor_modal.classList.remove("hidden");
}

if(confirm_no_btn){
    confirm_no_btn.addEventListener("click", () => {
        confirmation_modal.classList.add("hidden");
        confirmation_txt.textContent = ``;
        if (confirmHandler) {
            confirm_yes_btn.removeEventListener("click", confirmHandler);
        }
    });
}

if (close_modal_btn) close_modal_btn.addEventListener("click", () => {
    clearFields();
    if (warning_txt) warning_txt.classList.add("hidden");
    if (add_doctor_modal) add_doctor_modal.classList.add("hidden");
});

if (add_doctor_modal) add_doctor_modal.addEventListener("click", (e) => {
    if (e.target === add_doctor_modal) {
        clearFields();
        if (warning_txt) warning_txt.classList.add("hidden");
        if (add_doctor_modal) add_doctor_modal.classList.add("hidden");
    }
});

function toggleCustomVal(select){
    if (custom_shift_input) {
        (select.value == "custom-shift") ? custom_shift_input.classList.remove("hidden") : custom_shift_input.classList.add("hidden");
    }
}

if (max_shift_el) {
    max_shift_el.addEventListener('change', (e) => toggleCustomVal(e.target));
}

if (deleteButtonEl) {
    deleteButtonEl.addEventListener("click", () => {
        if (!currentEditingDoctorOriginalName) { return; }

        confirmation_modal.classList.remove("hidden");
        confirmation_txt.textContent = `Apakah Anda yakin untuk menghapus data dokter: ${currentEditingDoctorOriginalName}?`;

        if (confirmHandler) { confirm_yes_btn.removeEventListener("click", confirmHandler); }

        confirmHandler = async () => {
            try {
                const response = await window.update_doctor({
                    original_name: currentEditingDoctorOriginalName,
                    is_delete: true
                });

                clearFields();

                if (warning_txt) warning_txt.classList.add("hidden");
                if (add_doctor_modal) add_doctor_modal.classList.add("hidden");
            } catch (error) {
                console.error("JS Error during deletion process:", error);
                if (add_doctor_modal) add_doctor_modal.classList.remove("hidden");
            } finally {
                confirmation_modal.classList.add("hidden");
                confirmation_txt.textContent = ``;
                confirm_yes_btn.removeEventListener("click", confirmHandler);
            }
        };

        confirm_yes_btn.addEventListener("click", confirmHandler);
    });
}

if (saveButtonEl) saveButtonEl.addEventListener("click", async () => {
    const doctorDataFromForm = {
        name: (first_name_el.value + " " + last_name_el.value).trim(),
        birthdate: {
            day: birthdate_el.value.substring(0, 2),
            month: birthdate_el.value.substring(3,5),
            year: birthdate_el.value.substring(6)
        },
        phone: phone_el.value.trim(),
        address: address_el.value.trim(),
        specialty: specialty_el.value,
        max_shift: ((max_shift_el.value === "custom-shift") ? (custom_shift_input ? custom_shift_input.value.trim() : '') : max_shift_el.value),
        shift_preference: shift_preference_el.value,
        photo: ""
    };

    let field_is_empty = false;

    if (doctorDataFromForm.name.length === 0) field_is_empty = true;
    if (doctorDataFromForm.phone.length === 0) field_is_empty = true;
    if (doctorDataFromForm.address.length === 0) field_is_empty = true;

    if (doctorDataFromForm.birthdate.day.length === 0 ||
        doctorDataFromForm.birthdate.month.length === 0 ||
        doctorDataFromForm.birthdate.year.length === 0) 
    {
        field_is_empty = true;
    }

    if (doctorDataFromForm.specialty === "Pilih spesialisasi") field_is_empty = true;
    if (doctorDataFromForm.max_shift === "Pilih shift per minggu") field_is_empty = true;
    if (doctorDataFromForm.max_shift === "custom-shift" && (custom_shift_input ? custom_shift_input.value.trim().length === 0 : true)) {
        field_is_empty = true;
    }
    if (doctorDataFromForm.shift_preference === "Pilih preferensi shift") field_is_empty = true;
    
    if(field_is_empty){
        if (warning_txt) warning_txt.classList.remove("hidden");
        if (add_doctor_modal) add_doctor_modal.classList.remove("hidden");
    }
    else{
        try {
            let cResponse;
            if (currentEditingDoctorOriginalName) {
                const updatedDoctorData = {
                    original_name: currentEditingDoctorOriginalName,
                    ...doctorDataFromForm
                };
                cResponse = await window.update_doctor(updatedDoctorData); 
            } else {
                cResponse = await window.append_doctor(doctorDataFromForm);
                console.log(cResponse);
            }
            clearFields();
            if (warning_txt) warning_txt.classList.add("hidden");
            if (add_doctor_modal) add_doctor_modal.classList.add("hidden");

        }
        catch (error) {
            console.error("JS Error during save/update process:", error);
            if (add_doctor_modal) add_doctor_modal.classList.remove("hidden");
        }
    }
});

function clearFields(){
    if (first_name_el) first_name_el.value = "";
    if (last_name_el) last_name_el.value = "";
    if (birthdate_el) birthdate_el.value = "";
    if (phone_el) phone_el.value = "";
    if (address_el) address_el.value = "";
    if (specialty_el) specialty_el.value = "Pilih spesialisasi";
    if (max_shift_el) max_shift_el.value = "Pilih shift per minggu";
    if (custom_shift_input) custom_shift_input.classList.add("hidden");
    if (shift_preference_el) shift_preference_el.value = "Pilih preferensi shift";
    currentEditingDoctorOriginalName = null;
    if (deleteButtonEl) deleteButtonEl.classList.add("hidden");
}

async function loadDoctors(){
    if(doctors_list_container) doctors_list_container.innerHTML = '';

    if(window.load_doctors){
        try {
            const doctorsData = await window.load_doctors();

            doctors = doctorsData;
            if (Array.isArray(doctors)){
                let total_num_shift_pref_violation = 0;
                let total_doctors = 0;
                doctors.forEach((doctor) => {
                    console.log(doctor);
                    const card = createDoctorCard(doctor);
                    doctors_list_container.appendChild(card);
                    total_num_shift_pref_violation += doctor.preference_violation;
                    total_doctors += 1;
                });
                localStorage.setItem("total_doctors", total_doctors);
                document.getElementById("doctor-section-title").innerHTML = `Dokter (${total_doctors})`;
                document.getElementById("total-pref-violation-txt").innerHTML = `Pelanggaran preferensi total: ${total_num_shift_pref_violation}`;
            }
            else{
                console.error("JS Error: Expected array, got:", doctors);
            }
        } catch (error) {
            console.error("JS Error calling load_doctors:", error);
        }
    } else {
        console.error("Global load_doctors binding is undefined.");
    }
}

async function notifyDataUpdated() {
    await new Promise(resolve => setTimeout(resolve, 350));
    await loadDoctors();
}

async function getPersonnelPerShift(){
    let personnel_per_shift_data = null;
    document.getElementById("personnel-per-shift-txt").innerHTML = "";
    try{
        personnel_per_shift_data = await window.get_personnel_per_shift();
    }
    catch{
        console.log("Unable to get personnel per shift data from C");
    }

    if(personnel_per_shift_data !== null){
        const [morning, noon, night] = personnel_per_shift_data.split(";").map(n => n.trim());    
        document.getElementById("personnel-per-shift-txt").innerHTML = `Personel per shift: ${morning} (Pagi), ${noon} (Siang), ${night} (Malam)`;
    }
}

document.addEventListener("DOMContentLoaded", async () => {
    const loginModal = document.getElementById("login-modal");
    if (loginModal) {
        const auth_state = localStorage.getItem("auth-state");
        if (auth_state !== "logged_in") {
            loginModal.classList.remove("hidden");
        } else {
            loginModal.classList.add("hidden");
        }
    }

    await loadDoctors();
    await getPersonnelPerShift();
});


// Auth
const admin_el = document.getElementById("admin-name");
if (admin_el) admin_el.textContent = localStorage.getItem("name") || "Guest";

const uname_el = document.getElementById("uname");
const pass_el = document.getElementById("pwd");
const login_btn = document.getElementById("login-btn")
const logout_btn = document.getElementById("logout-btn")

if (login_btn) login_btn.addEventListener("click", () => {
    if (uname_el && pass_el && uname_el.value === "Admin" && pass_el.value === "Admin") {
        localStorage.setItem("name", uname_el.value);
        localStorage.setItem("auth-state", "logged_in");
        location.reload();
    }
    if (uname_el) uname_el.value = "";
    if (pass_el) pass_el.value = "";
});

if (logout_btn) logout_btn.addEventListener("click", () => {
    localStorage.setItem("auth-state", "logged_out");
    location.reload();
});
