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
            ((System.ComponentModel.ISupportInitialize)(this.videoImage)).BeginInit();
            this.SuspendLayout();
            // 
            // videoImage
            // 
            this.videoImage.Location = new System.Drawing.Point(12, 12);
            this.videoImage.Name = "videoImage";
            this.videoImage.Size = new System.Drawing.Size(871, 376);
            this.videoImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.videoImage.TabIndex = 0;
            this.videoImage.TabStop = false;
            // 
            // lblMemory
            // 
            this.lblMemory.Location = new System.Drawing.Point(799, 427);
            this.lblMemory.Name = "lblMemory";
            this.lblMemory.Size = new System.Drawing.Size(84, 13);
            this.lblMemory.TabIndex = 1;
            this.lblMemory.Text = "Unknown";
            this.lblMemory.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 11F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(895, 449);
            this.Controls.Add(this.lblMemory);
            this.Controls.Add(this.videoImage);
            this.Font = new System.Drawing.Font("Lucida Console", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "MainForm";
            this.Text = "Захват видео";
            ((System.ComponentModel.ISupportInitialize)(this.videoImage)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox videoImage;
        private System.Windows.Forms.Label lblMemory;
    }
}

