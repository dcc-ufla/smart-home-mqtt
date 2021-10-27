namespace MQTTBroker
{
    partial class BrokerForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.StartBrokerButton = new System.Windows.Forms.Button();
            this.StopBrokerButton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.PortTextBox = new wmgCMS.WaterMarkTextBox();
            this.SuspendLayout();
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(61, 4);
            // 
            // StartBrokerButton
            // 
            this.StartBrokerButton.BackColor = System.Drawing.SystemColors.Control;
            this.StartBrokerButton.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.StartBrokerButton.Location = new System.Drawing.Point(12, 27);
            this.StartBrokerButton.Name = "StartBrokerButton";
            this.StartBrokerButton.Size = new System.Drawing.Size(94, 36);
            this.StartBrokerButton.TabIndex = 1;
            this.StartBrokerButton.Text = "Start";
            this.StartBrokerButton.UseVisualStyleBackColor = false;
            this.StartBrokerButton.Click += new System.EventHandler(this.StartBrokerButton_Click);
            // 
            // StopBrokerButton
            // 
            this.StopBrokerButton.Location = new System.Drawing.Point(112, 27);
            this.StopBrokerButton.Name = "StopBrokerButton";
            this.StopBrokerButton.Size = new System.Drawing.Size(94, 36);
            this.StopBrokerButton.TabIndex = 2;
            this.StopBrokerButton.Text = "Stop";
            this.StopBrokerButton.UseVisualStyleBackColor = true;
            this.StopBrokerButton.Click += new System.EventHandler(this.StopBrokerButton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(519, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Port:";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(12, 87);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(467, 357);
            this.textBox1.TabIndex = 7;
            this.textBox1.Text = "Log...";
            // 
            // PortTextBox
            // 
            this.PortTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.PortTextBox.Location = new System.Drawing.Point(554, 15);
            this.PortTextBox.MaxLength = 4;
            this.PortTextBox.Name = "PortTextBox";
            this.PortTextBox.Size = new System.Drawing.Size(43, 20);
            this.PortTextBox.TabIndex = 5;
            this.PortTextBox.WaterMarkColor = System.Drawing.Color.Gray;
            this.PortTextBox.WaterMarkText = "1883";
            this.PortTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.PortTextBox_KeyPress);
            // 
            // BrokerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(609, 450);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.PortTextBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.StopBrokerButton);
            this.Controls.Add(this.StartBrokerButton);
            this.MaximumSize = new System.Drawing.Size(625, 489);
            this.MinimumSize = new System.Drawing.Size(625, 489);
            this.Name = "BrokerForm";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.Button StartBrokerButton;
        private System.Windows.Forms.Button StopBrokerButton;
        private System.Windows.Forms.Label label1;
        private wmgCMS.WaterMarkTextBox PortTextBox;
        private System.Windows.Forms.TextBox textBox1;
    }
}

