import threading
import subprocess

def upload_file(file_name):
    cmd = ['curl', '-F', f'file=@{file_name}', 'http://127.0.0.1:5555/cgi-bin/upload.py']
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(f"{file_name} yükleme sonucu:\n{result.stdout}\n{result.stderr}")

# Yüklenecek dosyaların listesi
files = ['dosya1.jpg', 'dosya2.jpg', 'dosya3.jpg', 'dosya4.jpg', 'dosya5.jpg']

threads = []

for file in files:
    t = threading.Thread(target=upload_file, args=(file,))
    threads.append(t)
    t.start()

# Tüm thread'lerin bitmesini bekle
for t in threads:
    t.join()

print("Tüm dosyalar yükleme işlemi tamamlandı.")
