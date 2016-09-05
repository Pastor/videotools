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
            ((System.ComponentModel.ISupportInitialize)(this.videoImage)).BeginInit();
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
            this.lblMemory.Location = new System.Drawing.Point(475, 426);
            this.lblMemory.Name = "lblMemory";
            this.lblMemory.Size = new System.Drawing.Size(65, 13);
            this.lblMemory.TabIndex = 1;
            this.lblMemory.Text = "Unknown";
            this.lblMemory.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnStart
            // 
            this.btnStart.Location = new System.Drawing.Point(12, 422);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(81, 23);
            this.btnStart.TabIndex = 2;
            this.btnStart.Text = "Запуск";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // btnComplete
            // 
            this.btnComplete.Location = new System.Drawing.Point(12, 393);
            this.btnComplete.Name = "btnComplete";
            this.btnComplete.Size = new System.Drawing.Size(81, 23);
            this.btnComplete.TabIndex = 3;
            this.btnComplete.Text = "Остановка";
            this.btnComplete.UseVisualStyleBackColor = true;
            this.btnComplete.Click += new System.EventHandler(this.btnComplete_Click);
            // 
            // lblInformation
            // 
            this.lblInformation.Location = new System.Drawing.Point(100, 427);
            this.lblInformation.Name = "lblInformation";
            this.lblInformation.Size = new System.Drawing.Size(369, 12);
            this.lblInformation.TabIndex = 4;
            // 
            // lblDetected
            // 
            this.lblDetected.Location = new System.Drawing.Point(100, 400);
            this.lblDetected.Name = "lblDetected";
            this.lblDetected.Size = new System.Drawing.Size(249, 10);
            this.lblDetected.TabIndex = 5;
            this.lblDetected.Text = "Unknown";
            // 
            // btnGC
            // 
            this.btnGC.Location = new System.Drawing.Point(553, 421);
            this.btnGC.Name = "btnGC";
            this.btnGC.Size = new System.Drawing.Size(33, 23);
            this.btnGC.TabIndex = 6;
            this.btnGC.Text = "GC";
            this.btnGC.UseVisualStyleBackColor = true;
            this.btnGC.Click += new System.EventHandler(this.btnGC_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 11F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(597, 448);
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
    }
}

