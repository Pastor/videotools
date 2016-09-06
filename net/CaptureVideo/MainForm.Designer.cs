namespace CaptureVideo
{
    partial class MainForm
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            this.videoImage = new System.Windows.Forms.PictureBox();
            this.lblMemory = new System.Windows.Forms.Label();
            this.btnStart = new System.Windows.Forms.Button();
            this.btnComplete = new System.Windows.Forms.Button();
            this.lblInformation = new System.Windows.Forms.Label();
            this.lblDetected = new System.Windows.Forms.Label();
            this.btnGC = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.pSurprise = new System.Windows.Forms.ProgressBar();
            this.pSorrow = new System.Windows.Forms.ProgressBar();
            this.pJoy = new System.Windows.Forms.ProgressBar();
            this.pFear = new System.Windows.Forms.ProgressBar();
            this.pDisgust = new System.Windows.Forms.ProgressBar();
            this.pAnger = new System.Windows.Forms.ProgressBar();
            this.pNone = new System.Windows.Forms.ProgressBar();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.videoImage)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // videoImage
            // 
            this.videoImage.Location = new System.Drawing.Point(12, 12);
            this.videoImage.Name = "videoImage";
            this.videoImage.Size = new System.Drawing.Size(574, 376);
            this.videoImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.videoImage.TabIndex = 0;
            this.videoImage.TabStop = false;
            // 
            // lblMemory
            // 
            this.lblMemory.Location = new System.Drawing.Point(732, 373);
            this.lblMemory.Name = "lblMemory";
            this.lblMemory.Size = new System.Drawing.Size(83, 15);
            this.lblMemory.TabIndex = 1;
            this.lblMemory.Text = "1000Т";
            this.lblMemory.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnStart
            // 
            this.btnStart.Location = new System.Drawing.Point(732, 172);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(81, 23);
            this.btnStart.TabIndex = 2;
            this.btnStart.Text = "Запуск";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // btnComplete
            // 
            this.btnComplete.Location = new System.Drawing.Point(645, 172);
            this.btnComplete.Name = "btnComplete";
            this.btnComplete.Size = new System.Drawing.Size(81, 23);
            this.btnComplete.TabIndex = 3;
            this.btnComplete.Text = "Остановка";
            this.btnComplete.UseVisualStyleBackColor = true;
            this.btnComplete.Click += new System.EventHandler(this.btnComplete_Click);
            // 
            // lblInformation
            // 
            this.lblInformation.Location = new System.Drawing.Point(592, 211);
            this.lblInformation.Name = "lblInformation";
            this.lblInformation.Size = new System.Drawing.Size(223, 14);
            this.lblInformation.TabIndex = 4;
            this.lblInformation.Text = "Информация";
            // 
            // lblDetected
            // 
            this.lblDetected.Location = new System.Drawing.Point(592, 198);
            this.lblDetected.Name = "lblDetected";
            this.lblDetected.Size = new System.Drawing.Size(223, 13);
            this.lblDetected.TabIndex = 5;
            this.lblDetected.Text = "Определение лица";
            // 
            // btnGC
            // 
            this.btnGC.Location = new System.Drawing.Point(592, 172);
            this.btnGC.Name = "btnGC";
            this.btnGC.Size = new System.Drawing.Size(33, 23);
            this.btnGC.TabIndex = 6;
            this.btnGC.Text = "GC";
            this.btnGC.UseVisualStyleBackColor = true;
            this.btnGC.Click += new System.EventHandler(this.btnGC_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.pSurprise);
            this.groupBox1.Controls.Add(this.pSorrow);
            this.groupBox1.Controls.Add(this.pJoy);
            this.groupBox1.Controls.Add(this.pFear);
            this.groupBox1.Controls.Add(this.pDisgust);
            this.groupBox1.Controls.Add(this.pAnger);
            this.groupBox1.Controls.Add(this.pNone);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(592, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(221, 154);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Напряженное состояние";
            // 
            // pSurprise
            // 
            this.pSurprise.Location = new System.Drawing.Point(95, 126);
            this.pSurprise.Name = "pSurprise";
            this.pSurprise.Size = new System.Drawing.Size(114, 16);
            this.pSurprise.Step = 1;
            this.pSurprise.TabIndex = 13;
            // 
            // pSorrow
            // 
            this.pSorrow.Location = new System.Drawing.Point(95, 108);
            this.pSorrow.Name = "pSorrow";
            this.pSorrow.Size = new System.Drawing.Size(114, 16);
            this.pSorrow.Step = 1;
            this.pSorrow.TabIndex = 12;
            // 
            // pJoy
            // 
            this.pJoy.Location = new System.Drawing.Point(95, 90);
            this.pJoy.Name = "pJoy";
            this.pJoy.Size = new System.Drawing.Size(114, 16);
            this.pJoy.Step = 1;
            this.pJoy.TabIndex = 11;
            // 
            // pFear
            // 
            this.pFear.Location = new System.Drawing.Point(95, 72);
            this.pFear.Name = "pFear";
            this.pFear.Size = new System.Drawing.Size(114, 16);
            this.pFear.Step = 1;
            this.pFear.TabIndex = 10;
            // 
            // pDisgust
            // 
            this.pDisgust.Location = new System.Drawing.Point(95, 54);
            this.pDisgust.Name = "pDisgust";
            this.pDisgust.Size = new System.Drawing.Size(114, 16);
            this.pDisgust.Step = 1;
            this.pDisgust.TabIndex = 9;
            // 
            // pAnger
            // 
            this.pAnger.Location = new System.Drawing.Point(95, 36);
            this.pAnger.Name = "pAnger";
            this.pAnger.Size = new System.Drawing.Size(114, 16);
            this.pAnger.Step = 1;
            this.pAnger.TabIndex = 8;
            // 
            // pNone
            // 
            this.pNone.Location = new System.Drawing.Point(95, 18);
            this.pNone.Name = "pNone";
            this.pNone.Size = new System.Drawing.Size(114, 16);
            this.pNone.Step = 1;
            this.pNone.TabIndex = 7;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(7, 132);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(80, 10);
            this.label7.TabIndex = 6;
            this.label7.Text = "Удивление";
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(6, 114);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(80, 10);
            this.label6.TabIndex = 5;
            this.label6.Text = "Печаль";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(6, 96);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(80, 10);
            this.label5.TabIndex = 4;
            this.label5.Text = "Радость";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(7, 78);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(80, 10);
            this.label4.TabIndex = 3;
            this.label4.Text = "Страх";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(6, 60);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(80, 10);
            this.label3.TabIndex = 2;
            this.label3.Text = "Отвращение";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(7, 42);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 10);
            this.label2.TabIndex = 1;
            this.label2.Text = "Гнев";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(7, 21);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(35, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Нет";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 11F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(823, 399);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnGC);
            this.Controls.Add(this.lblDetected);
            this.Controls.Add(this.lblInformation);
            this.Controls.Add(this.btnComplete);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.lblMemory);
            this.Controls.Add(this.videoImage);
            this.Font = new System.Drawing.Font("Lucida Console", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "MainForm";
            this.Text = "Захват видео";
            ((System.ComponentModel.ISupportInitialize)(this.videoImage)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox videoImage;
        private System.Windows.Forms.Label lblMemory;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.Button btnComplete;
        private System.Windows.Forms.Label lblInformation;
        private System.Windows.Forms.Label lblDetected;
        private System.Windows.Forms.Button btnGC;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ProgressBar pNone;
        private System.Windows.Forms.ProgressBar pSurprise;
        private System.Windows.Forms.ProgressBar pSorrow;
        private System.Windows.Forms.ProgressBar pJoy;
        private System.Windows.Forms.ProgressBar pFear;
        private System.Windows.Forms.ProgressBar pDisgust;
        private System.Windows.Forms.ProgressBar pAnger;
    }
}

