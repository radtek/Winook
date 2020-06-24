﻿using System;
using System.Diagnostics;
using System.Windows;
using Winook;

namespace Winook.Desktop.Core.Test
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private MouseHook _mouseHook;
        private KeyboardHook _keyboardHook;
        private Process _process;
        private bool _mouseHookInstalled;
        private bool _keyboardHookInstalled;

        public MainWindow()
        {
            InitializeComponent();
            if (!Environment.Is64BitOperatingSystem)
            {
                radio64bit.IsEnabled = false;
            }
        }

        private async void mouseButton_Click(object sender, RoutedEventArgs e)
        {
            if (!_mouseHookInstalled)
            {
                if (_process == null || _process.HasExited)
                {
                    if (Environment.Is64BitOperatingSystem && (radio32bit.IsChecked ?? false))
                    {
                        _process = Process.Start(@"c:\windows\syswow64\notepad.exe");
                    }
                    else
                    {
                        _process = Process.Start(@"c:\windows\notepad.exe");
                    }
                }

                if (ignoreMove.IsChecked ?? false)
                {
                    _mouseHook = new MouseHook(_process.Id, MouseMessageTypes.IgnoreMove);
                }
                else
                {
                    _mouseHook = new MouseHook(_process.Id);
                }

                _mouseHook.MessageReceived += MouseHook_MessageReceived;
                _mouseHook.LeftButtonUp += MouseHook_LeftButtonUp;
                _mouseHook.AddHandler(MouseMessageCode.NCLeftButtonUp, MouseHook_NCLButtonUp);
                _mouseHook.RemoveHandler(MouseMessageCode.NCLeftButtonUp, MouseHook_NCLButtonUp);
                mouseButton.Content = "Installing hook...";
                await _mouseHook.InstallAsync();
                _mouseHookInstalled = true;
                mouseButton.Content = "Mouse Unhook";
            }
            else
            {
                _mouseHook.Uninstall();
                _mouseHookInstalled = false;
                mouseButton.Content = "Mouse Hook";
            }
        }

        private void MouseHook_LeftButtonUp(object sender, MouseMessageEventArgs e)
        {
            testLabel.Dispatcher.BeginInvoke(new Action(() =>
            {
                testLabel.Content = $"Code: {e.MessageCode}; X: {e.X}; Y: {e.Y}; Delta: {e.Delta}";
            }));
        }

        private void MouseHook_MessageReceived(object sender, MouseMessageEventArgs e)
        {
            mouseLabel.Dispatcher.BeginInvoke(new Action(() =>
            {
                mouseLabel.Content = $"Code: {e.MessageCode}; X: {e.X}; Y: {e.Y}; Delta: {e.Delta}";
            }));
        }

        private void MouseHook_NCLButtonUp(object sender, MouseMessageEventArgs e)
        {
            testLabel.Dispatcher.BeginInvoke(new Action(() =>
            {
                testLabel.Content = $"Code: {e.MessageCode}; X: {e.X}; Y: {e.Y}; Delta: {e.Delta}";
            }));
        }

        private async void keyboardButton_Click(object sender, EventArgs e)
        {
            if (!_keyboardHookInstalled)
            {
                if (_process == null || _process.HasExited)
                {
                    if (Environment.Is64BitOperatingSystem && (radio32bit.IsChecked ?? false))
                    {
                        _process = Process.Start(@"c:\windows\syswow64\notepad.exe");
                    }
                    else
                    {
                        _process = Process.Start(@"c:\windows\notepad.exe");
                    }
                }

                _keyboardHook = new KeyboardHook(_process.Id);
                _keyboardHook.MessageReceived += KeyboardHook_MessageReceived;
                //_keyboardHook.AddHandler(KeyCode.Y, KeyboardHook_Test);
                _keyboardHook.AddHandler(KeyCode.Y, Modifiers.ControlShift, KeyboardHook_Test);
                _keyboardHook.AddHandler(KeyCode.U, Modifiers.Shift | Modifiers.RightControl, KeyboardHook_Test);
                _keyboardHook.AddHandler(KeyCode.N, Modifiers.AltControlShift, KeyboardHook_Test);
                _keyboardHook.AddHandler(KeyCode.T, KeyboardHook_Test);
                keyboardButton.Content = "Installing hook...";
                await _keyboardHook.InstallAsync();
                _keyboardHookInstalled = true;
                keyboardButton.Content = "Keyboard Unhook";
            }
            else
            {
                _keyboardHook.Uninstall();
                _keyboardHookInstalled = false;
                keyboardButton.Content = "Keyboard Hook";
            }
        }

        private void KeyboardHook_MessageReceived(object sender, KeyboardMessageEventArgs e)
        {
            keyboardLabel.Dispatcher.BeginInvoke(new Action(() =>
            {
                keyboardLabel.Content = $"Code: {e.KeyValue}; Modifiers: {e.Modifiers}; Flags: {e.Flags:x} "
                    + $"Shift: {e.Shift}; Control: {e.Control}; Alt: {e.Alt}; Direction: {e.Direction}";
            }));
        }

        private void KeyboardHook_Test(object sender, KeyboardMessageEventArgs e)
        {
            Debug.Write($"KeyboardHook_Test - Code: {e.KeyValue}; Modifiers: {e.Modifiers:x}; Flags: {e.Flags:x}; ");
            Debug.WriteLine($"Shift: {e.Shift}; Control: {e.Control}; Alt: {e.Alt}; Direction: {e.Direction}");
            testLabel.Dispatcher.BeginInvoke(new Action(() =>
            {
                testLabel.Content = $"Code: {e.KeyValue}; Modifiers: {e.Modifiers}; Flags: {e.Flags:x} "
                    + $"Shift: {e.Shift}; Control: {e.Control}; Alt: {e.Alt}; Direction: {e.Direction}";
            }));
        }
    }
}
