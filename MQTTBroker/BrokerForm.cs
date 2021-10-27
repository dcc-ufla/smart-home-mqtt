using System;
using System.Drawing;
using System.Windows.Forms;
using MQTTnet;
using MQTTnet.Protocol;
using MQTTnet.Server;

namespace MQTTBroker
{
    /// <summary>
    /// Classe referente ao servidor/broker
    /// </summary>
    public partial class BrokerForm : Form
    {
        // Variaveis
        #region Variaveis

            /// <summary>
            /// Instancia do servidor
            /// </summary>
            public IMqttServer _MqttServer;

            /// <summary>
            /// Porta que o servidor ira operar
            /// </summary>
            public string _Port = "1883";
        #endregion

        // Construtor
        #region Construtor

            /// <summary>
            /// Construtor
            /// </summary>
            public BrokerForm()
            {
                // Inicialização da tela
                InitializeComponent();

                // Linka evento do foco para o botão da porta
                this.PortTextBox.LostFocus += this.PortTextBox_LostFocus;
            }

        #endregion

        // Metodos Privados
        #region Metodos Privados

            // Metodos de controle das regras
            #region Metodos de Regra

                /// <summary>
                /// Metodo para lidar com os botões de ativação e desativação do broker
                /// </summary>
                /// <param name="sender"></param>
                /// <param name="e"></param>
                private async void StartBrokerButton_Click(object sender, EventArgs e)
                {
                    // Indica atraves da cor que o servidor está ligado
                    this.StartBrokerButton.BackColor = Color.ForestGreen;

                    // Caso instancia do servidor não esteja vazia
                    if (this._MqttServer != null)
                    {
                        // Retorna vazio
                        return;
                    }

                    //JsonServerStorage storage = new JsonServerStorage();
                    //storage.Clear();

                    // Cria instancia do servidor
                    this._MqttServer = new MqttFactory().CreateMqttServer();

                    // Cria instancia da configuração do servidor
                    MqttServerOptions options = new MqttServerOptions();

                    // Adiciona porta a configuração do servidor
                    options.DefaultEndpointOptions.Port = int.Parse(this._Port);
                    //options.Storage = storage;

                    // Classifica como sessão persistente
                    options.EnablePersistentSessions = true;

                    // Configurações de validação de clientes
                    options.ConnectionValidator = new MqttServerConnectionValidatorDelegate(
                    c =>
                    {
                        // Caso o ID do cliente seja maior que 10
                        if (c.ClientId.Length < 10)
                        {
                            // Rejeita por indentificação invalida
                            c.ReasonCode = MqttConnectReasonCode.ClientIdentifierNotValid;
                            return;
                        }

                        if (c.Username != "username")
                        {
                            c.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
                            return;
                        }

                        if (c.Password != "password")
                        {
                            c.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
                            return;
                        }

                        c.ReasonCode = MqttConnectReasonCode.Success;
                    });

                    try
                    {
                        // Inicia servidor
                        await this._MqttServer.StartAsync(options);
                    }
                    catch (Exception ex)
                    {
                        // Caso aconteça algum erro, reportar
                        MessageBox.Show(ex.Message, "Error Occurs", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        await this._MqttServer.StopAsync();
                        this._MqttServer = null;
                    }

                }

                /// <summary>
                /// Controla a cor do botão de start do server quando o servidor for desligado
                /// </summary>
                /// <param name="sender"></param>
                /// <param name="e"></param>
                private async void StopBrokerButton_Click(object sender, EventArgs e)
                {
                    // Ao desligar deixa a cor padrão
                    this.StartBrokerButton.BackColor = DefaultBackColor;

                    // Caso não exista instancia para ser parada
                    if (this._MqttServer == null)
                    {
                        // Retorna
                        return;
                    }

                    // Para instancia do servidor
                    await this._MqttServer.StopAsync();
                    this._MqttServer = null;
                }

            #endregion

            // Metodos de controle do formulario
            #region Controle Formulario
                /// <summary>
                /// Metodo para lidar com a perca do foco
                /// </summary>
                /// <param name="sender"></param>
                /// <param name="e"></param>
                private void PortTextBox_LostFocus(object sender, System.EventArgs e)
                {
                    // Caso o tamanho do texto seja de 4 para uma porta
                    if (this.PortTextBox.Text.Length == 4)
                    {
                        // Muda o valor da porta
                        this._Port = this.PortTextBox.Text;
                    }
                    // Caso não seja de tamanho 4 o valor da porta
                    else
                    {
                        // Apaga
                        this.PortTextBox.Text = "";
                    }
                }

                /// <summary>
                /// Metodo para lidar com caracteres indesejaveis no campo da porta
                /// </summary>
                /// <param name="sender"></param>
                /// <param name="e"></param>
                private void PortTextBox_KeyPress(object sender, KeyPressEventArgs e)
                {
                    // Se o caracter for numeros, aceita
                    if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar))
                    {
                        // Aceita
                        e.Handled = true;
                    }
                }

            #endregion

        #endregion
    }
}
