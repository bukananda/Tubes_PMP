let current_date = new Date();
let current_month = current_date.getMonth();
let current_year = current_date.getFullYear();

let prev_saved_start_date = null;

const calendar_grid = document.getElementById("calendar-grid");
const selected_date = document.getElementById("selected-date");
const select_date_warning_txt = document.getElementById("select-date-warning-txt");
const auto_schedule_btn = document.getElementById("auto-schedule-btn");

let prev_selected_cell = null;
let current_panel = null;

const doc_number_morning_el = document.getElementById("doc-number-morning");
const doc_number_noon_el = document.getElementById("doc-number-noon");
const doc_number_night_el = document.getElementById("doc-number-night");

const select_week_option = document.getElementById("select-week-option");
let days_in_week_txt_el = [];
const morning_row = document.getElementById("morning-row");
const noon_row = document.getElementById("noon-row");
const night_row = document.getElementById("night-row");

function formatDateToDDMMYYYY(year, month, day) {
    const d = String(day).padStart(2, '0');
    const m = String(month + 1).padStart(2, '0');
    const y = String(year);
    return `${d}-${m}-${y}`;
}

function parseDDMMYYYY(dateString) {
    const parts = dateString.split('-');
    if (parts.length === 3) {
        const day = parseInt(parts[0], 10);
        const month = parseInt(parts[1], 10) - 1;
        const year = parseInt(parts[2], 10);
        return new Date(year, month, day);
    }
    return null;
}

function get_day_date(start_date, nth_week) {
  const dayNames = ["Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"];

  const results = [];

  const baseDate = new Date(start_date);
  baseDate.setDate(baseDate.getDate() + (nth_week - 1) * 7);

  for (let i = 0; i < 7; i++) {
    const currentDate = new Date(baseDate);
    currentDate.setDate(baseDate.getDate() + i);

    const day = String(currentDate.getDate()).padStart(2, "0");
    const month = String(currentDate.getMonth() + 1).padStart(2, "0");
    const year = currentDate.getFullYear();
    const formattedDate = `${day}-${month}-${year}`;

    results.push({
      date: formattedDate,
      day_name: dayNames[currentDate.getDay()]
    });
  }

  return results;
}

async function getPrevStartDate(){
    try {
        prev_saved_start_date = await window.get_prev_start_date();
        console.log(prev_saved_start_date);
    }
    catch (error){
        console.log("failed to get prev_saved_start_date");
    }
}

async function renderWeeklyShiftTable(selectedWeekValue) {
    // Clear existing cells
    morning_row.querySelectorAll('td:not(:first-child)').forEach(td => td.remove());
    noon_row.querySelectorAll('td:not(:first-child)').forEach(td => td.remove());
    night_row.querySelectorAll('td:not(:first-child)').forEach(td => td.remove());

    const weekNumber = parseInt(selectedWeekValue.replace('week-', ''), 10);
    const date_day_names = get_day_date(parseDDMMYYYY(prev_saved_start_date), weekNumber);

    let weeklyShiftData = { week_data: [] };

    if (!prev_saved_start_date || prev_saved_start_date === "UNSET") {
        console.warn("WARNING: Schedule start date not set. Cannot render weekly shift table.");
        for (let i = 0; i < 7; i++) {
             const morningCell = document.createElement("td");
             const noonCell = document.createElement("td");
             const nightCell = document.createElement("td");
             const commonCellClasses = ["h-[72px]", "px-4", "py-2", "w-[400px]", "text-[#638488]", "sm:text-xs", "lg:text-sm", "font-normal", "leading-normal", "break-words"];
             morningCell.classList.add(...commonCellClasses);
             noonCell.classList.add(...commonCellClasses);
             nightCell.classList.add(...commonCellClasses);
             morningCell.textContent = '—';
             noonCell.textContent = '—';
             nightCell.textContent = '—';
             morning_row.appendChild(morningCell);
             noon_row.appendChild(noonCell);
             night_row.appendChild(nightCell);

            if (days_in_week_txt_el[i]) {
                days_in_week_txt_el[i].querySelector("p:first-child").textContent = 'N/A';
                days_in_week_txt_el[i].querySelector("p:last-child").textContent = '(N/A)';
            }
        }
        return; 
    }

    try{
        weeklyShiftData = await window.load_weekly_schedule(weekNumber.toString());
        //If wrong format received
        if (!weeklyShiftData || !Array.isArray(weeklyShiftData.week_data)){ weeklyShiftData = { week_data: [] }; }
    }
    catch(error){
        //Guard error
        weeklyShiftData = { week_data: [] };
    }

    for (let i = 0; i < 7; i++) {
        const dayData = weeklyShiftData.week_data[i];

        if (date_day_names[i] && date_day_names[i].day_name !== "N/A") {
            if (days_in_week_txt_el[i]) {
                days_in_week_txt_el[i].querySelector("p:first-child").textContent = `${date_day_names[i].day_name}`;
                days_in_week_txt_el[i].querySelector("p:last-child").textContent = `(${date_day_names[i].date})`;
            }
        } else {
            // For days beyond the 30-day schedule (e.g., in week 5), display N/A in headers by default
            if (days_in_week_txt_el[i]) {
                days_in_week_txt_el[i].querySelector("p:first-child").textContent = 'N/A';
                days_in_week_txt_el[i].querySelector("p:last-child").textContent = '(N/A)';
            }
        }

        const morningCell = document.createElement("td");
        const noonCell = document.createElement("td");
        const nightCell = document.createElement("td");

        const commonCellClasses = ["h-[72px]", "px-4", "py-2", "w-[400px]", "text-[#638488]", "sm:text-xs", "lg:text-sm", "font-medium", "leading-normal", "break-words"];

        morningCell.classList.add(...commonCellClasses);
        noonCell.classList.add(...commonCellClasses);
        nightCell.classList.add(...commonCellClasses);

        if (dayData && dayData.shifts) {
            morningCell.innerHTML = dayData.shifts.pagi.map((name, idx, arr) => ((idx !== arr.length - 1) ? `<div>dr. ${name},</div>` : `<div>dr. ${name}</div>`)).join('') || '&mdash;';
            noonCell.innerHTML = dayData.shifts.siang.map((name, idx, arr) => ((idx !== arr.length - 1) ? `<div>dr. ${name},</div>` : `<div>dr. ${name}</div>`)).join('') || '&mdash;';
            nightCell.innerHTML = dayData.shifts.malam.map((name, idx, arr) => ((idx !== arr.length - 1) ? `<div>dr. ${name},</div>` : `<div>dr. ${name}</div>`)).join('') || '&mdash;';
        } else {
            morningCell.textContent = '—';
            noonCell.textContent = '—';
            nightCell.textContent = '—';
        }

        morning_row.appendChild(morningCell);
        noon_row.appendChild(noonCell);
        night_row.appendChild(nightCell);
    }
}

function renderCalendar(){
    // Calendar Generation
    const date_txt = document.getElementById("date-text");
    const months = ["Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"];

    if(current_month < 0){
        current_month = 11;
        current_year -= 1;
    }
    else if(current_month > 11){
        current_month = 0;
        current_year += 1;
    }

    date_txt.textContent = `${months[current_month]} ${current_year.toString()}`

    let firstDayOfMonth = new Date(current_year, current_month, 1).getDay();
    let lastDateOfMonth = new Date(current_year, current_month + 1, 0).getDate();
    let lastDayOfMonth = new Date(current_year, current_month, lastDateOfMonth).getDay();
    let lastDateOfLastMonth = new Date(current_year, current_month, 0).getDate();

    if (calendar_grid) { calendar_grid.innerHTML = ''; }

    for(let i = firstDayOfMonth; i > 0; i--){
        const day = document.createElement("button");
        day.classList.add("calendar-day", "h-20", "w-full", "bg-[#f0f4f4]", "text-[#A6ABB2]", "text-sm", "font-medium", "leading-normal", "border-b-[1.5px]");
        if(i == firstDayOfMonth) day.classList.add("col-start-1");

        const day_num = lastDateOfLastMonth - i + 1;
        const prevMonth = (current_month === 0) ? 11 : current_month - 1;
        const prevYear = (current_month === 0) ? current_year - 1 : current_year;
        
        day.dataset.fullDate = formatDateToDDMMYYYY(prevYear, prevMonth, day_num);

        const day_txt = document.createElement("div");
        day_txt.classList.add("flex", "size-full", "items-center", "justify-center", "rounded-full");
        day_txt.textContent = (lastDateOfLastMonth - i + 1);
        day.appendChild(day_txt);
        calendar_grid.appendChild(day);
    }

    const today = new Date();
    for(let i = 1; i <= lastDateOfMonth; i++){
        const day = document.createElement("button");
        day.classList.add("calendar-day", "h-20", "w-full", "text-[#111718]", "text-sm", "font-medium", "leading-normal", "border-b-[1.5px]");
        
        day.dataset.fullDate = formatDateToDDMMYYYY(current_year, current_month, i);
        
        const day_txt = document.createElement("div");
        day_txt.classList.add("flex", "size-full", "items-center", "justify-center", "rounded-full");
        day_txt.textContent = (i);
        if (i === today.getDate() && current_month === today.getMonth() && current_year === today.getFullYear()) day.classList.add("bg-[#2196f3]", "bg-opacity-20");
        day.appendChild(day_txt);
        calendar_grid.appendChild(day);
    }

    for(let i = 1; i <= (6 - lastDayOfMonth); i++){
        const day = document.createElement("button");
        day.classList.add("calendar-day", "h-20", "w-full", "bg-[#f0f4f4]", "text-[#A6ABB2]", "text-sm", "font-medium", "leading-normal", "border-b-[1.5px]");
        
        const nextMonth = (current_month === 11) ? 0 : current_month + 1;
        const nextYear = (current_month === 11) ? current_year + 1 : current_year;

        day.dataset.fullDate = formatDateToDDMMYYYY(nextYear, nextMonth, i);
        
        const day_txt = document.createElement("div");
        day_txt.classList.add("flex", "size-full", "items-center", "justify-center", "rounded-full");
        day_txt.textContent = (i);
        day.appendChild(day_txt);
        calendar_grid.appendChild(day);
    }

    // Calendar events
    const dateCells = document.querySelectorAll('.calendar-day');

    dateCells.forEach(cell => {
        const cell_date_obj = parseDDMMYYYY(cell.dataset.fullDate);
        cell_date_obj.setHours(0,0,0,0);
        if(prev_saved_start_date !== "UNSET"){
            const prev_sd_obj = parseDDMMYYYY(prev_saved_start_date);
            prev_sd_obj.setHours(0,0,0,0);
            const day_diff = Math.floor((cell_date_obj.getTime()-prev_sd_obj.getTime())/(1000 * 60 * 60 * 24));
            if(day_diff >= 0 && day_diff < 30){
                cell.classList.add("bg-[#4883C0]", "bg-opacity-35")
            }
        }

        cell.addEventListener('click', async (e) => {
            e.stopPropagation();

            if (cell === prev_selected_cell) return;

            if (prev_selected_cell) {
                prev_selected_cell.classList.remove("bg-[#125DC0]", "bg-opacity-45");
            }

            if (current_panel) {
                current_panel.remove();
                current_panel = null;
            }

            const clickedDateString = cell.dataset.fullDate;
            const clickedDateString_obj = parseDDMMYYYY(clickedDateString);
            clickedDateString_obj.setHours(0,0,0,0);

            let daily_shift_data = null;
            var n_docs_on_shift = 1; //Default 1
            var docs_on_shift = [];
            await getPrevStartDate();
            if(prev_saved_start_date !== "UNSET"){
                const prev_saved_start_date_obj = parseDDMMYYYY(prev_saved_start_date);
                prev_saved_start_date_obj.setHours(0,0,0,0);

                const clickedDateString_obj_time = clickedDateString_obj.getTime();
                const prev_saved_start_date_obj_time = prev_saved_start_date_obj.getTime();

                if(clickedDateString_obj_time >= prev_saved_start_date_obj_time && (Math.abs(prev_saved_start_date_obj_time-clickedDateString_obj_time)/((1000 * 60 * 60 * 24)) < 30)){
                    cell.classList.add("bg-[#125DC0]", "bg-opacity-45");
                    try {
                        daily_shift_data = await window.load_daily_schedule(clickedDateString);
                        
                        if (daily_shift_data && daily_shift_data.shifts) {
                            const docs_morning = daily_shift_data.shifts.pagi || [];
                            const docs_noon = daily_shift_data.shifts.siang || [];
                            const docs_night = daily_shift_data.shifts.malam || [];

                            const floating_panel = document.createElement("div");
                            floating_panel.classList.add("absolute", "h-fit", "transform", "-translate-x-[160px]", "w-[400px]", "rounded-lg", "border", "border-gray-300", "bg-white", "text-[#111718]", "p-4", "shadow-lg", "z-50");
                            const panel_title = document.createElement("h1");
                            panel_title.textContent = `Jadwal shift untuk ${clickedDateString}`;
                            panel_title.classList.add("font-bold", "text-lg", "mb-2");
                            floating_panel.appendChild(panel_title);

                            // For three shifts
                            for(let i = 0; i < 3; i++){
                                const shift_el = document.createElement("div");
                                shift_el.classList.add("flex", "flex-row", "w-full", "mx-6", "py-2", "justify-start", "items-center");
                                if(i !== 2) shift_el.classList.add("border-b");
                                const shift_title = document.createElement("p");
                                shift_title.classList.add("flex", "w-1/3");
                                
                                switch(i){
                                    case 0:
                                        shift_title.textContent = "Shift Pagi: ";
                                        n_docs_on_shift = docs_morning.length;
                                        docs_on_shift = docs_morning;
                                        break;
                                    case 1:
                                        shift_title.textContent = "Shift Siang: ";
                                        n_docs_on_shift = docs_noon.length;
                                        docs_on_shift = docs_noon;
                                        break;
                                    case 2:
                                        shift_title.textContent = "Shift Malam: ";
                                        n_docs_on_shift = docs_night.length;
                                        docs_on_shift = docs_night;
                                        break;
                                    default:
                                        break;
                                }
                                const shift_data = document.createElement("ol");
                                shift_data.classList.add("flex", "flex-col", "items-start", "list-decimal", "list-inside");

                                for(let i_doc = 0; i_doc < n_docs_on_shift; i_doc++){
                                    const docs_in_list = document.createElement("li");
                                    docs_in_list.textContent = "dr. " + docs_on_shift[i_doc];
                                    shift_data.appendChild(docs_in_list);
                                }

                                shift_el.appendChild(shift_title);
                                shift_el.appendChild(shift_data);
                                floating_panel.appendChild(shift_el);
                            }

                            const info_txt = document.createElement("p");
                            info_txt.classList.add("flex", "justify-self-center", "font-thin", "text-xs", "text-gray-800");
                            info_txt.textContent = "Klik luar untuk menutup";
                            floating_panel.appendChild(info_txt);

                            cell.appendChild(floating_panel);
                            current_panel = floating_panel;
                            prev_selected_cell = cell;

                            current_panel.classList.remove('hidden');
                        }  
                        else{
                            console.warn("Error: No valid shift data found from C for:", clickedDateString, daily_shift_data);
                        }
                    }
                    catch(error){
                        console.error("Error: Error fetching daily shifts:", error);
                    }
                }
            }
            else{
                console.log("Cancelled : ", prev_saved_start_date);
            }
        });

        cell.addEventListener('contextmenu', (e) => {
            e.preventDefault();
            
            today.setHours(0,0,0,0);

            const rightClickedDateString = cell.dataset.fullDate;
            const rightClickedDateObj = parseDDMMYYYY(cell.dataset.fullDate);
            rightClickedDateObj.setHours(0,0,0,0);

            if (selected_date){
                if(rightClickedDateObj.getTime() >= today.getTime()){
                    selected_date.value = rightClickedDateString;
                    select_date_warning_txt.classList.add("hidden");
                }
                else{
                    select_date_warning_txt.textContent = "*Tidak dapat memilih waktu lampau!";
                    select_date_warning_txt.classList.remove("hidden");
                }
            }
        });
    });
}

const cal_swap_left_btn = document.getElementById("calendar-swap-left-btn");
const cal_swap_right_btn = document.getElementById("calendar-swap-right-btn");

if (cal_swap_left_btn) {
    cal_swap_left_btn.addEventListener("click", () => {
        current_month -= 1;
        updateCalendar();
    });
}

if (cal_swap_right_btn) {
    cal_swap_right_btn.addEventListener("click", () => {
        current_month += 1;
        updateCalendar();
    });
}

function resetDates(){
    if (calendar_grid) {
        while (calendar_grid.children.length > 1) {
            calendar_grid.removeChild(calendar_grid.lastElementChild);
        }
    }
}

//Update Calendar
function updateCalendar(){
    resetDates();
    renderCalendar();
}

if (auto_schedule_btn) {
    auto_schedule_btn.addEventListener("click", async (e) => {
        e.stopPropagation();
        const start_date = selected_date.value;
        const doc_number_morning = parseInt(doc_number_morning_el.value);
        const doc_number_noon = parseInt(doc_number_noon_el.value);
        const doc_number_night = parseInt(doc_number_night_el.value);

        if (selected_date.value === "" || selected_date.value === selected_date.placeholder ||
            isNaN(doc_number_morning) || isNaN(doc_number_noon) || isNaN(doc_number_night) || doc_number_morning < 0 || doc_number_noon < 0 || doc_number_night < 0)
        {
            select_date_warning_txt.textContent = "*Mohon mengisi semua kolom dan memilih tanggal dengan klik kanan di kalender!";
            select_date_warning_txt.classList.remove("hidden");
            return;
        }
        else if(parseInt(localStorage.getItem("total_doctors")) == 0){
            select_date_warning_txt.textContent = "*Tidak ada dokter yang tersedia!";
            select_date_warning_txt.classList.remove("hidden");
            return;
        }

        select_date_warning_txt.classList.add("hidden");
        auto_schedule_btn.disabled = true;
        
        try {
            const response = await window.generate_schedule({
                start_date: start_date,
                doc_number_morning: doc_number_morning,
                doc_number_noon: doc_number_noon,
                doc_number_night: doc_number_night
            });

            await getPrevStartDate();
            updateCalendar();

            if (select_week_option.value && select_week_option.value !== "Pilih minggu") {
                renderWeeklyShiftTable(select_week_option.value);
            } else {
                renderWeeklyShiftTable("week-1"); //Default week 1
            }
            
        } catch (error) {
            console.error("Error: JS Error calling generate_schedule:", error);
        } finally {
            auto_schedule_btn.disabled = false;
        }
    });
} else {
    console.error("Error: 'auto_schedule_btn' element not found!");
}

// Auth and Initial Load
document.addEventListener("DOMContentLoaded", () => {
    //Auth
    const auth_state = localStorage.getItem("auth-state");
    if (auth_state !== "logged_in") { window.location.replace("main.html"); }

    const days_in_week_txt_eid = [
        "first-day-txt",
        "second-day-txt",
        "third-day-txt",
        "fourth-day-txt",
        "fifth-day-txt",
        "sixth-day-txt",
        "seventh-day-txt"
    ];
    days_in_week_txt_el = days_in_week_txt_eid.map(id => document.getElementById(id));

    document.addEventListener('click', (event) => {
        if (current_panel &&
            !current_panel.contains(event.target) &&
            !prev_selected_cell.contains(event.target)
        ){
            current_panel.remove();
            prev_selected_cell.classList.remove("bg-[#125DC0]", "bg-opacity-45");
            current_panel = null;
            prev_selected_cell = null;
        }
        if(selected_date && !selected_date.contains(event.target)) {
            selected_date.value = "";
            selected_date.placeholder = "Klik kanan pada tanggal di kalender";
            select_date_warning_txt.classList.add("hidden");
        }
    });

    getPrevStartDate().then(() => {
        renderCalendar();
        
        if (select_week_option){
            select_week_option.addEventListener('change', (e) => {
                renderWeeklyShiftTable(e.target.value);
            });
            select_week_option.value = "week-1";
            renderWeeklyShiftTable("week-1");
        }
    });
});


let admin_name = "";
const admin = document.getElementById("admin-name");
if (admin) {
    admin.textContent = localStorage.getItem("name") || "Guest";
}


const logout_btn = document.getElementById("logout-btn");
if (logout_btn) {
    logout_btn.addEventListener("click", () => {
        localStorage.setItem("auth-state", "logged_out");
        location.reload();
    });
}