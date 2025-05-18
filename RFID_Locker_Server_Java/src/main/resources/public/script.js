const headers = {
    users: ['ID', 'Imię i nazwisko', 'Typ'],
    locks: ['ID', 'Opis', 'Piętro'],
    logs: ['Czas', 'Użytkownik ID', 'Imię i nazwisko', 'Pokój', 'Karta OK?', 'Dostęp?'],
    privileges: ['User ID', 'Imię i nazwisko', 'ID Zamka', 'Opis pokoju']
};

function loadData(type) {
    const formWrapper = document.getElementById("addUserFormWrapper");
    formWrapper.style.display = type === 'users' ? 'block' : 'none';
    const roomFormWrapper = document.getElementById("addRoomFormWrapper");
    roomFormWrapper.style.display = (type === "locks") ? "block" : "none";
    const privilegeFormWrapper = document.getElementById("addPrivilegeFormWrapper");
    privilegeFormWrapper.style.display = (type === "privileges") ? "block" : "none";

    if (type === "privileges") {
        loadUsersForSelect();
        loadLocksForSelect();
    }


    fetch('/api/' + type)
        .then(r => r.json())
        .then(data => {
            const thead = document.querySelector("#dataTable thead");
            const tbody = document.querySelector("#dataTable tbody");
            thead.innerHTML = "<tr>" + headers[type].map(h => `<th>${h}</th>`).join('') + "</tr>";
            tbody.innerHTML = data.map(row => {
                const isDenied = (row[5] === "NIE" && type === "logs");
                return `<tr class="${isDenied ? "danger" : ""}">` +
                    row.map(cell => `<td>${cell}</td>`).join('') +
                    "</tr>";
            }).join('');
        });
}

document.getElementById("addUserForm").addEventListener("submit", function(e) {
    e.preventDefault();
    const formData = new FormData(this);
    fetch('/api/addUser', {
        method: 'POST',
        body: new URLSearchParams(formData)
    }).then(response => {
        if (response.ok) {
            this.reset();
            loadData('users');
        } else {
            alert("Błąd przy dodawaniu użytkownika");
        }
    });
});

document.getElementById("addRoomForm").addEventListener("submit", function(e) {
    e.preventDefault();

    const formData = new FormData(this);
    const params = new URLSearchParams(formData);

    fetch('/api/addRoom', {
        method: 'POST',
        body: params
    }).then(r => {
        if (r.ok) {
            loadData('locks'); // odśwież tabelę z pokojami
            this.reset();
        } else {
            alert("Błąd przy dodawaniu pokoju");
        }
    });
});

// Funkcje do pobrania użytkowników i zamków i uzupełnienia selectów
function loadUsersForSelect() {
    fetch('/api/users')
        .then(res => res.json())
        .then(users => {
            const userSelect = document.getElementById('userSelect');
            userSelect.innerHTML = '<option value="" disabled selected>Wybierz użytkownika</option>';
            users.forEach(user => {
                // Zakładam, że user to [user_id, name, type]
                const option = document.createElement('option');
                option.value = user[0];
                option.textContent = user[1];
                userSelect.appendChild(option);
            });
        });
}

function loadLocksForSelect() {
    fetch('/api/locks')
        .then(res => res.json())
        .then(locks => {
            const lockSelect = document.getElementById('lockSelect');
            lockSelect.innerHTML = '<option value="" disabled selected>Wybierz pokój</option>';
            locks.forEach(lock => {
                // Zakładam, że lock to [lock_id, description, floor]
                const option = document.createElement('option');
                option.value = lock[0];
                option.textContent = lock[1];
                lockSelect.appendChild(option);
            });
        });
}

// Obsługa submit formularza
document.getElementById('addPrivilegeForm').addEventListener('submit', function(e) {
    e.preventDefault();
    const formData = new FormData(this);
    const params = new URLSearchParams(formData);

    fetch('/api/addPrivilege', {
        method: 'POST',
        body: params
    }).then(r => {
        if (r.ok) {
            loadData('privileges'); // odśwież tabelę uprawnień
            this.reset();
        } else {
            alert('Błąd przy dodawaniu uprawnienia');
        }
    });
});




loadData('logs');

// Co 5 sekund odświeżaj aktywną zakładkę
setInterval(() => {
    const active = document.querySelector("nav a.active");
    if (active) {
        const type = active.getAttribute("onclick").match(/'(.+?)'/)[1];
        loadData(type);
    }
}, 5000); // co 5000 ms = 5 sek