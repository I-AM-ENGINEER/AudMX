



class IcomManager():
    def process_folder(self, folder_path: str) -> list[bytes]:
        """
        #читает иконки из папки и прогоняет их через преобразование
        :param folder_path: - относительный путь к папку и иконками
        :return: массив байтовых строк
        """
        byte_arrays = []

        for filename in os.listdir(folder_path):

            if filename.endswith(".bmp"):
                if (len(self.volLevelApp) > 4):
                    return byte_arrays

                file_path = os.path.join(folder_path, filename)

                byte_array = self.bmp_to_byte_array(file_path)
                if (len(byte_array) == 352):
                    if str(filename[8:-4]) + '.exe' in self.open_process_list:
                        self.volLevelApp.append([str(filename[8:-4]), True])
                        byte_arrays.append(byte_array)
                else:
                    self.trayIcon.masegeIconWarning(str(filename[8:-4]))
        return byte_arrays

    def bmp_to_byte_array(self, image_path: str) -> bytes:
        """
        #преобразование картинок в байт массив
        :param image_path: - путь к иконками
        :return: байт массив
        """
        img = Image.open(image_path)
        if img.mode != '1':
            raise ValueError("Изображение не является монохромным")
        img_bytes = img.tobytes()
        #/print("bmp_to_byte_array: ", img_bytes)
        return img_bytes
    def loadIconOnESP(self, ans=0):
        self.num_load_icon = self.num_load_icon + ans
        if (self.num_load_icon == 5):
            self.num_load_icon = 0
            self.mas_icon.clear()
            return
        self.ser.writeSerial("SET_ICON " + str(self.mas_icon[self.num_load_icon][1]) + "\n")
        self.timer_loadByte.start()

    def handleGetIcon(self):
        self.timer_loadByte.start()
    def loadByteMasToESP(self):
        self.teat_perer += 1
        #/print(len(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64]) , self.teat_perer, self.num_load_icon)
        self.ser.writeByteSerial(self.mas_icon[self.num_load_icon][0][(self.teat_perer - 1) * 64:self.teat_perer * 64])
        if (self.teat_perer == 6):
            self.timer_loadByte.stop()
            self.teat_perer = 0
            self.num_load_icon += 1
